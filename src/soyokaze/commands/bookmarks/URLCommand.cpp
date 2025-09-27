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

CString URLCommand::GetGuideString()
{
	return _T("⏎:ブラウザで開く S-⏎:URLをクリップボードにコピー");
}

CString URLCommand::GetTypeDisplayName()
{
	CString product;
	in->mEnv->GetProductName(product);
	return TypeDisplayName(product);
}

BOOL URLCommand::Execute(Parameter* param)
{
	if (GetModifierKeyState(param, MASK_SHIFT) != 0) {
		// URLをクリップボードにコピー
		Clipboard::Copy(in->mBookmarkItem.mUrl);
		return TRUE;
	}

	// URLをブラウザで開く
	CString path;
	if (in->mEnv->GetInstalledExePath(path) == false) {
		CString msg(_T("Browser executable not found."));
		msg += _T("\n");
		msg += (LPCTSTR)path;
		AfxMessageBox(msg);
		return FALSE;
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	exec.Run((LPCTSTR)path, in->mBookmarkItem.mUrl, process);

	return TRUE;
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
