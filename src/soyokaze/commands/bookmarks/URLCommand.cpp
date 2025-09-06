#include "pch.h"
#include "framework.h"
#include "URLCommand.h"
#include "commands/common/SubProcess.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
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

CString URLCommand::GetGuideString()
{
	return _T("⏎:ブラウザで開く S-⏎:URLをクリップボードにコピー");
}

CString URLCommand::GetTypeDisplayName()
{
	static CString TEXT_BOOKMARK((LPCTSTR)IDS_COMMAND_BOOKMARK);

	CString str;
	str.Format(_T("%s %s"), (LPCTSTR)Bookmark::GetBrowserName(in->mBrowserType), (LPCTSTR)TEXT_BOOKMARK);

	return str;
}

BOOL URLCommand::Execute(Parameter* param)
{
	if (GetModifierKeyState(param, MASK_SHIFT) != 0) {
		// URLをクリップボードにコピー
		Clipboard::Copy(in->mBookmarkItem.mUrl);
		return TRUE;
	}

	// URLをブラウザで開く
	Path path;
	if (Bookmark::GetExecutablePath(in->mBrowserType, path, path.size()) == false) {
		return FALSE;
	}

	if (path.FileExists() == false) {
		CString msg(_T("Browser executable not found."));
		msg += _T("\n");
		msg += (LPCTSTR)path;
		AfxMessageBox(msg);
		return TRUE;
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	exec.Run((LPCTSTR)path, in->mBookmarkItem.mUrl, process);

	return TRUE;
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

}}} // end of namespace launcherapp::commands::bookmarks
