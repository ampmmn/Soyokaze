#include "pch.h"
#include "ChromeEnvironment.h"
#include "utility/Path.h"
#include "utility/VersionInfo.h"

namespace launcherapp { namespace externaltool { namespace webbrowser {

// Chrome $USERPROFILE/AppData/Local/Google/Chrome/User Data/Default/Bookmarks
#define USERDATA_PATH _T("Google\\Chrome\\User Data")
#define BOOKMARK_PATH _T("\\Default\\Bookmarks")
#define HISTORY_PATH _T("\\Default\\History")

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
	Path bookmarkFilePath{Path::LOCALAPPDATA, USERDATA_PATH BOOKMARK_PATH};
	path = (LPCTSTR)bookmarkFilePath;
	return true;
}

// 履歴ファイルのパスを取得
bool ChromeEnvironment::GetHistoryFilePath(CString& path)
{
	Path bookmarkFilePath{Path::LOCALAPPDATA, USERDATA_PATH HISTORY_PATH};
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

