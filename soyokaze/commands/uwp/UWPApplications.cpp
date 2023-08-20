#include "pch.h"
#include "UWPApplications.h"
#include "utility/RegistryKey.h"
#include <mutex>
#include <thread>
#include <deque>

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
	RegistryKey HKCR(HKEY_CLASSES_ROOT);

	TCHAR path[MAX_PATH_NTFS];
	_tcscpy_s(path, _T("Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\PackageRepository\\Extensions\\windows.protocol"));

	std::vector<CString> schemeNames;
	HKCR.EnumSubKeyNames(path, schemeNames);

	LPTSTR p = path + _tcslen(path);
	p[0] = _T('\\');
	p++;


	CString appNamePath;
	CString defaultIconPath;
	CString appName;
	CString iconPath;
	std::vector<CString> subKeys;

	std::vector<ITEM> tmp;
	for (auto& scheme : schemeNames) {

		Sleep(0);

		_tcscpy_s(p, 16383, scheme);

		RegistryKey subKey;
		HKCR.OpenSubKey(path, subKey);

		subKey.EnumSubKeyNames(subKeys);
		if (subKeys.empty()) {
			continue;
		}

		const CString& appIdentifier = subKeys[0];

		appNamePath.Format(_T("%s\\Application"),
		                   appIdentifier);

		defaultIconPath.Format(_T("%s\\DefaultIcon"), 
		                   appIdentifier);

		HKCR.GetValue(appNamePath, _T("ApplicationName"), appName);
		HKCR.GetValue(defaultIconPath, _T(""), iconPath);

		// 文字列を解決する
		if (appName.Find(_T("@{")) == 0)  {
			TCHAR buff[MAX_PATH_NTFS];
			SHLoadIndirectString(appName, buff, MAX_PATH_NTFS, nullptr);
			appName = buff;
		}
		if (iconPath.Find(_T("@{")) == 0)  {
			TCHAR buff[MAX_PATH_NTFS];
			SHLoadIndirectString(iconPath, buff, MAX_PATH_NTFS, nullptr);
 			iconPath = buff;
		}

		// アプリ名/アイコンパス/scheme名からITEMを生成する
		ITEM item;
		item.mName = appName;
		item.mDescription = appName;
		item.mIconPath = iconPath;
		item.mScheme = scheme;

		//TRACE(_T("Name:%s Scheme:%s\n"), appName, scheme);

		tmp.push_back(item);
	}
	items.swap(tmp);
}

} // end of namespace uwp
} // end of namespace commands
} // end of namespace soyokaze

