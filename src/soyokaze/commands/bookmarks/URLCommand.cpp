#include "pch.h"
#include "framework.h"
#include "URLCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/web/OpenURLAction.h"
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
	BrowserEnvironment* mEnv{nullptr};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(URLCommand)

URLCommand::URLCommand(
	const Bookmark& item,
	BrowserEnvironment* brwsEnv
) : 
	AdhocCommandBase(item.mName, item.mName),
	in(std::make_unique<PImpl>())
{
	in->mBookmarkItem = item;
	in->mEnv = brwsEnv;
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
	CString product;
	in->mEnv->GetProductName(product);
	return TypeDisplayName(product);
}

bool URLCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	auto modifierFlags = hotkeyAttr.GetModifiers();
	if (modifierFlags == 0) {
		*action = new actions::web::OpenURLAction(in->mBookmarkItem.mUrl, in->mEnv);
		return true;
	}
	else if (modifierFlags == MOD_SHIFT) {
		*action = new actions::clipboard::CopyTextAction(in->mBookmarkItem.mUrl);
		return true;
	}
	return false;
}

HICON URLCommand::GetIcon()
{
	// ブラウザに応じたアイコンを取得
	CString path;
	if (in->mEnv->GetInstalledExePath(path)) {
		return IconLoader::Get()->LoadIconFromPath(path);
	}
	else {
		return IconLoader::Get()->LoadWebIcon();
	}
}

launcherapp::core::Command*
URLCommand::Clone()
{
	return new URLCommand(in->mBookmarkItem, in->mEnv);
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

CString URLCommand::TypeDisplayName(LPCTSTR productName)
{
	static CString TEXT_BOOKMARK((LPCTSTR)IDS_COMMAND_BOOKMARK);

	CString str;
	str.Format(_T("%s %s"), (LPCTSTR)TEXT_BOOKMARK, productName);

	return str;
}


}}} // end of namespace launcherapp::commands::bookmarks
