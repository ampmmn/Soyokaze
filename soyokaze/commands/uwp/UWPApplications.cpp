#include "pch.h"
#include "UWPApplications.h"
#include "utility/RegistryKey.h"
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


namespace soyokaze {
namespace commands {
namespace uwp {

static const int INTERVAL = 10000;

struct UWPApplications::PImpl
{
	std::mutex mMutex;
	std::vector<ITEM> mItems;
	DWORD mElapsed = 0;
	bool mIsUpdated = false;
};


UWPApplications::UWPApplications() : in(std::make_unique<PImpl>())
{
}

UWPApplications::~UWPApplications()
{
}

bool UWPApplications::GetApplications(std::vector<ITEM>& items)
{
	{
		std::lock_guard<std::mutex> lock(in->mMutex);
		DWORD elapsed = GetTickCount() - in->mElapsed;
		if (elapsed <= INTERVAL) {
			if (in->mIsUpdated) {
				items = in->mItems;
				in->mIsUpdated = false;
				return true;
			}
			return false;
		}
	}

	std::thread th([&]() {
		
		CoInitialize(NULL);

		std::vector<ITEM> tmp;
		EnumApplications(tmp);

		CoUninitialize();

		std::lock_guard<std::mutex> lock(in->mMutex);
		in->mItems.swap(tmp);
		in->mElapsed = GetTickCount();
		in->mIsUpdated = true;
	});
	th.detach();

	std::lock_guard<std::mutex> lock(in->mMutex);
	if (in->mIsUpdated) {
		items = in->mItems;
		in->mIsUpdated = false;
		return true;
	}
	return false;
}

void UWPApplications::EnumApplications(std::vector<ITEM>& items)
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

	std::vector<ITEM> tmpList;

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
		if (SUCCEEDED(hr) && value.vt != VT_EMPTY) {
			continue;
		}

		LPWSTR strVal = nullptr;
		hr = item->GetDisplayName(SIGDN_NORMALDISPLAY, &strVal);

		CString dispName(strVal);
		CoTaskMemFree(strVal);

		hr = propStore->GetValue(modelIdKey, &value);
		if (FAILED(hr)) {
			continue;
		}
		CString appId;
		hr = PropVariantToString(value, appId.GetBuffer(1024), 1024);
		appId.ReleaseBuffer();

		PIDLIST_ABSOLUTE pidl;
		SHFILEINFO sfi = {};

		hr = SHGetIDListFromObject((IUnknown*)item, &pidl);
    if (FAILED(hr)) {
			continue;
		}

		SHGetFileInfo(reinterpret_cast<LPCTSTR>(pidl), 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ICON);
		CoTaskMemFree(pidl);

		ITEM newItem;
		newItem.mName = dispName;
		newItem.mDescription = dispName;
		newItem.mAppID = appId;
		newItem.mIcon = sfi.hIcon;

		tmpList.push_back(newItem);
	}
	items.swap(tmpList);
}

} // end of namespace uwp
} // end of namespace commands
} // end of namespace soyokaze

