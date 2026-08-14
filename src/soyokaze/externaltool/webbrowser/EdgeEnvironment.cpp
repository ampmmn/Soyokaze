#include "pch.h"
#include "EdgeEnvironment.h"
#include "BrowserProfile.h"
#include "utility/Path.h"

namespace launcherapp { namespace externaltool { namespace webbrowser {

#define USERDATA_PATH _T("Microsoft\\Edge\\User Data")


EdgeEnvironment* EdgeEnvironment::GetInstance()
{
	static EdgeEnvironment inst;
	return &inst;
}

bool EdgeEnvironment::IsAvailable()
{
	CString dummyPath;
	return GetInstalledExePath(dummyPath);
}

bool EdgeEnvironment::GetInstalledExePath(CString& path)
{
	Path path_;
	size_t reqLen = 0;
	_tgetenv_s(&reqLen, (LPTSTR)path_, path_.size(), _T("PROGRAMFILES"));
	PathAppend(path_, _T("Microsoft\\Edge\\Application\\msedge.exe"));
	if (path_.FileExists() == false) {
		// PROGRAMFILE以下で見つからなかったらPROGRAMFILES(X86)も試す
		_tgetenv_s(&reqLen, (LPTSTR)path_, path_.size(), _T("PROGRAMFILES(X86)"));
		PathAppend(path_, _T("Microsoft\\Edge\\Application\\msedge.exe"));
		if (path_.FileExists() == false) {
			return false;
		}
	}

	path = (LPCTSTR)path_;

	return true;
}

// 実行パラメータを取得する
bool EdgeEnvironment::GetCommandlineParameter(CString& param)
{
	param = _T("$target");
	return true;
}


// ブックマークデータのパスを取得
bool EdgeEnvironment::GetBookmarkFilePath(CString& path)
{
	Path bookmarkFilePath{Path::LOCALAPPDATA, USERDATA_PATH};
	CString profilePath;
	profilePath.Format(_T("%s\\Bookmarks"), (LPCTSTR)BrowserProfile::GetLastUsedProfileName(bookmarkFilePath));
	bookmarkFilePath.Append(profilePath);
	path = (LPCTSTR)bookmarkFilePath;
	return true;
}

// 履歴ファイルのパスを取得
bool EdgeEnvironment::GetHistoryFilePath(CString& path)
{
	Path bookmarkFilePath{Path::LOCALAPPDATA, USERDATA_PATH};
	CString profilePath;
	profilePath.Format(_T("%s\\History"), (LPCTSTR)BrowserProfile::GetLastUsedProfileName(bookmarkFilePath));
	bookmarkFilePath.Append(profilePath);
	path = (LPCTSTR)bookmarkFilePath;
	return true;
}

// 製品名を取得
bool EdgeEnvironment::GetProductName(CString& name)
{
	name = _T("Edge");
	return true;
}

}}}

