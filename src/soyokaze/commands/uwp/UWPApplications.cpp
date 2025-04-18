#include "pch.h"
#include "UWPApplications.h"
#include "utility/RegistryKey.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include <atlbase.h>
#include <propvarutil.h>
#include <mutex>
#include <thread>
#include <deque>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "propsys.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace uwp {

static constexpr int UPDATE_INTERVAL = 3600;      // 3600回*50msec = 180,000msec → 3分

struct UWPApplications::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void SetAbort()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mIsAbort = true;
	}
	bool IsAbort()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsAbort;
	}
	void RunUpdateTask();
	void EnumApplications(std::vector<ItemPtr>& items);

	void OnAppFirstBoot() override 
	{
		OnAppNormalBoot();
	}
	void OnAppNormalBoot() override
 	{
		// 一定間隔で更新をするタスクを起動
		RunUpdateTask();
	}
	void OnAppPreferenceUpdated() override {}
	void OnAppExit() override {}

	std::mutex mMutex;
	std::vector<ItemPtr> mItems;
	bool mIsAbort{false};
	CEvent mWaitEvt;
};

void UWPApplications::PImpl::RunUpdateTask()
{
	std::thread th([&]() {

		mWaitEvt.ResetEvent();

		Sleep(2000);
		HRESULT hr = CoInitialize(NULL);
		if (FAILED(hr)) {
			SPDLOG_ERROR(_T("Failed to CoInitialize!"));
		}

		bool isFirst = true;
		int count = UPDATE_INTERVAL;
		while(IsAbort() == false) {
			if (count < UPDATE_INTERVAL) {
				count++;
				Sleep(50);
				continue;
			}

			SharedHwnd hwnd;
			if (IsWindow(hwnd.GetHwnd()) == FALSE) {
				// 入力欄がクローズされたら終了
				break;
			}
			if (isFirst == false && IsWindowVisible(hwnd.GetHwnd())) {
				// 入力欄が表示されているときは更新しない
				Sleep(50);
				continue;
			}
			// カウンタをリセット
			count = 0;
			isFirst = false;

			std::vector<ItemPtr> tmp;
			EnumApplications(tmp);


			std::lock_guard<std::mutex> lock(mMutex);
			mItems = tmp;
		}

		mWaitEvt.SetEvent();
		CoUninitialize();
	});
	th.detach();
}

void UWPApplications::PImpl::EnumApplications(std::vector<ItemPtr>& items)
{
	HRESULT hr;

	PROPERTYKEY modelIdKey;
	hr = PSGetPropertyKeyFromName(_T("System.AppUserModel.ID"), &modelIdKey);
	PROPERTYKEY tppKey;
	hr = PSGetPropertyKeyFromName(_T("System.Link.TargetParsingPath"), &tppKey);

	CComPtr<IKnownFolderManager> pManager;
	hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pManager));
	if (FAILED(hr)) {
		return ;
	}

	CComPtr<IKnownFolder> pKnownFolder;
	hr = pManager->GetFolder(FOLDERID_AppsFolder, &pKnownFolder);
	if (FAILED(hr)) {
		return ;
	}

	CComPtr<IShellItem> appsFolder;
	hr = pKnownFolder->GetShellItem(0, IID_IShellItem, (void**)&appsFolder);
	if (FAILED(hr)) {
		return ;
	}

	CComPtr<IEnumShellItems> appItems;
	hr = appsFolder->BindToHandler(nullptr, BHID_StorageEnum, IID_IEnumShellItems, (void**)&appItems);

	std::vector<ItemPtr> tmpList;

	for(;;) {

		CComPtr<IShellItem> item;
		hr = appItems->Next(1, &item, nullptr);
		if (FAILED(hr) || hr == S_FALSE) {
			break;
		}

		CComPtr<IPropertyStore> propStore;
		hr = item->BindToHandler(nullptr, BHID_PropertyStore, IID_IPropertyStore, (void**)&propStore);
		if (FAILED(hr)) {
			continue;
		}

		PROPVARIANT value;
		hr = propStore->GetValue(tppKey, &value);
		if (FAILED(hr)) {
			continue;
		}

		// System.Link.TargetParsingPathがないものはUWPアプリ、そうでないものは非UWPアプリとして扱う
		bool isUWP = false;
		CString appId;
		if (value.vt != VT_EMPTY) {
			// 非UWPアプリ
			appId = value.bstrVal;
		}
		else {
			// UWPアプリ
			isUWP = true;
			hr = propStore->GetValue(modelIdKey, &value);
			if (FAILED(hr)) {
				continue;
			}
			hr = PropVariantToString(value, appId.GetBuffer(1024), 1024);
			appId.ReleaseBuffer();

			if (appId.Find(_T("::{"))==0) {
				// GUIDで始まるものは除外(PC/コントロールパネル/ファイル名を指定して実行)
				continue;
			}
		}

		// 表示名を取得
		LPWSTR strVal = nullptr;
		hr = item->GetDisplayName(SIGDN_NORMALDISPLAY, &strVal);

		CString dispName(strVal);
		CoTaskMemFree(strVal);


		PIDLIST_ABSOLUTE pidl;
		SHFILEINFO sfi = {};

		hr = SHGetIDListFromObject((IUnknown*)item, &pidl);
		if (FAILED(hr)) {
			continue;
		}

		SHGetFileInfo(reinterpret_cast<LPCTSTR>(pidl), 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ICON);
		CoTaskMemFree(pidl);

		auto newItem = std::make_shared<ITEM>(isUWP, dispName, appId, sfi.hIcon);
		tmpList.push_back(newItem);
	}
	items.swap(tmpList);
}



UWPApplications::UWPApplications() : in(std::make_unique<PImpl>())
{
	in->mWaitEvt.SetEvent();
}

UWPApplications::~UWPApplications()
{
	// 更新をするタスクの完了を待つ
	in->SetAbort();
	WaitForSingleObject(in->mWaitEvt, 5000);
}

bool UWPApplications::GetApplications(std::vector<ItemPtr>& items)
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	items = in->mItems;
	return true;
}

} // end of namespace uwp
} // end of namespace commands
} // end of namespace launcherapp

