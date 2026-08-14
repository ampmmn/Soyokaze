#include "pch.h"
#include "ChromeEnvironment.h"
#include "BrowserProfile.h"
#include "utility/Path.h"
#include "utility/VersionInfo.h"

namespace launcherapp { namespace externaltool { namespace webbrowser {

// Chromeのユーザーデータディレクトリ
#define USERDATA_PATH _T("Google\\Chrome\\User Data")

ChromeEnvironment* ChromeEnvironment::GetInstance()
{
	static ChromeEnvironment inst;
	return &inst;
}

bool ChromeEnvironment::IsAvailable()
{
	CString dummyPath;
	return GetInstalledExePath(dummyPath);
}

bool ChromeEnvironment::GetInstalledExePath(CString& path)
{
	Path path_;
	size_t reqLen = 0;
	_tgetenv_s(&reqLen, (LPTSTR)path_, path_.size(), _T("PROGRAMFILES"));
	PathAppend((LPTSTR)path_, _T("Google\\Chrome\\Application\\chrome.exe"));
	if (path_.FileExists() == false) {
		return false;
	}

	path = (LPCTSTR)path_;

	return true;
}

// 実行パラメータを取得する
bool ChromeEnvironment::GetCommandlineParameter(CString& param)
{
	param = _T("$target");
	return true;
}

// ブックマークデータのパスを取得
bool ChromeEnvironment::GetBookmarkFilePath(CString& path)
{
	Path bookmarkFilePath{Path::LOCALAPPDATA, USERDATA_PATH};
	CString profilePath;
	profilePath.Format(_T("%s\\Bookmarks"), (LPCTSTR)BrowserProfile::GetLastUsedProfileName(bookmarkFilePath));
	bookmarkFilePath.Append(profilePath);
	path = (LPCTSTR)bookmarkFilePath;
	return true;
}

// 履歴ファイルのパスを取得
bool ChromeEnvironment::GetHistoryFilePath(CString& path)
{
	Path bookmarkFilePath{Path::LOCALAPPDATA, USERDATA_PATH};
	CString profilePath;
	profilePath.Format(_T("%s\\History"), (LPCTSTR)BrowserProfile::GetLastUsedProfileName(bookmarkFilePath));
	bookmarkFilePath.Append(profilePath);
	path = (LPCTSTR)bookmarkFilePath;
	return true;
}

// 製品名を取得
bool ChromeEnvironment::GetProductName(CString& name)
{
	name = _T("Chrome");
	return true;
}

}}}

