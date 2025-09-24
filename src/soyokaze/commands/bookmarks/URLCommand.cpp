#include "pch.h"
#include "framework.h"
#include "URLCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/web/OpenInChromeAction.h"
#include "actions/web/OpenInEdgeAction.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "utility/Path.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp { namespace commands { namespace bookmarks {

struct URLCommand::PImpl
{
	Bookmark mBookmarkItem;
	BrowserType mBrowserType{BrowserType::Unknown};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(URLCommand)

URLCommand::URLCommand(
	const Bookmark& item,
	BrowserType browserType
) : 
	AdhocCommandBase(item.mName, item.mName),
	in(std::make_unique<PImpl>())
{
	in->mBookmarkItem = item;
	in->mBrowserType = browserType;
}

URLCommand::~URLCommand()
{
}

CString URLCommand::GetDescription()
{
	return in->mBookmarkItem.mFolderPath;
}

CString URLCommand::GetTypeDisplayName()
{
	return TypeDisplayName(in->mBrowserType);
}

bool URLCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		if (in->mBrowserType == BrowserType::Chrome) {
			*action = new actions::web::OpenInChromeAction(in->mBookmarkItem.mUrl);
			return true;
		}
		else if (in->mBrowserType == BrowserType::Edge) {
			*action = new actions::web::OpenInEdgeAction(in->mBookmarkItem.mUrl);
			return true;
		}
		else {
			return false;
		}
	}
	else if (modifierFlags == Command::MODIFIER_SHIFT) {
		*action = new actions::clipboard::CopyTextAction(in->mBookmarkItem.mUrl);
		return true;
	}
	return false;
}

HICON URLCommand::GetIcon()
{
	// ブラウザに応じたアイコンを取得
	Path path;
	Bookmark::GetExecutablePath(in->mBrowserType, path, path.size());
	return IconLoader::Get()->LoadIconFromPath((LPCTSTR)path);
}

launcherapp::core::Command*
URLCommand::Clone()
{
	return new URLCommand(in->mBookmarkItem, in->mBrowserType);
}

CString URLCommand::GetSourceName()
{
	return mName;
}

bool URLCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_EXTRACANDIDATE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidate*)this;
		return true;
	}
	return false;
}

CString URLCommand::TypeDisplayName(int type)
{
	static CString TEXT_BOOKMARK((LPCTSTR)IDS_COMMAND_BOOKMARK);

	CString str;
	str.Format(_T("%s %s"), (LPCTSTR)Bookmark::GetBrowserName((BrowserType)type), (LPCTSTR)TEXT_BOOKMARK);

	return str;
}


}}} // end of namespace launcherapp::commands::bookmarks
