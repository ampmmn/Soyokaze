#include "pch.h"
#include "BookmarkItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace bookmarks {

CString Bookmark::GetBrowserName()
{
	return mBrowser == BrowserType::Chrome ? _T("Chrome") : _T("Edge");
}

static bool GetChromeExecutablePath(LPTSTR path, size_t len)
{
	UNREFERENCED_PARAMETER(len);

	size_t reqLen = 0;
	_tgetenv_s(&reqLen, path, len, _T("PROGRAMFILES"));
	PathAppend(path, _T("Google\\Chrome\\Application\\chrome.exe"));

	return true;
}

static bool GetEdgeExecutablePath(LPTSTR path, size_t len)
{
	UNREFERENCED_PARAMETER(len);

	size_t reqLen = 0;
#ifndef _WIN64
	_tgetenv_s(&reqLen, path, len, _T("ProgramFiles"));
#else
	_tgetenv_s(&reqLen, path, len, _T("ProgramFiles(x86)"));
#endif
	PathAppend(path, _T("Microsoft\\Edge\\Application\\msedge.exe"));

	return true;
}

bool Bookmark::GetExecutablePath(LPTSTR path, size_t len)
{
	if (mBrowser == BrowserType::Chrome) {
		return GetChromeExecutablePath(path, len);
	}
	else if (mBrowser == BrowserType::Edge) {
		return GetEdgeExecutablePath(path, len);
	}
	else {
		return false;
	}
}


}}} // end of namespace launcherapp::commands::bookmarks

