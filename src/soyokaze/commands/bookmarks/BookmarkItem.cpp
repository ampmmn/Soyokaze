#include "pch.h"
#include "BookmarkItem.h"
#include "actions/web/OpenInChromeAction.h"
#include "actions/web/OpenInEdgeAction.h"

using namespace launcherapp::actions::web;

namespace launcherapp { namespace commands { namespace bookmarks {

CString Bookmark::GetBrowserName(BrowserType type)
{
	return type == BrowserType::Chrome ? _T("Chrome") : _T("Edge");
}

bool Bookmark::GetExecutablePath(BrowserType type, LPTSTR path, size_t len)
{
	if (type == BrowserType::Chrome) {
		return OpenInChromeAction::GetChromeExecutablePath(path, len);
	}
	else if (type == BrowserType::Edge) {
		return OpenInEdgeAction::GetEdgeExecutablePath(path, len);
	}
	else {
		return false;
	}
}


}}} // end of namespace launcherapp::commands::bookmarks

