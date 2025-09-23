#include "pch.h"
#include "WebHistoryAdhocCommand.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/web/OpenInChromeAction.h"
#include "actions/web/OpenInEdgeAction.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "utility/Path.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "framework.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using namespace launcherapp::actions::web;

namespace launcherapp {
namespace commands {
namespace webhistory {

struct WebHistoryAdhocCommand::PImpl
{
	bool GetExecutablePath(LPTSTR path, size_t len);

	CString mName;
	HISTORY mHistory;
};

bool WebHistoryAdhocCommand::PImpl::GetExecutablePath(LPTSTR path, size_t len)
{
	if (mHistory.mBrowserName == _T("Chrome")) {
		return OpenInChromeAction::GetChromeExecutablePath(path, len);
	}
	else if (mHistory.mBrowserName == _T("Edge")) {
		return OpenInEdgeAction::GetEdgeExecutablePath(path, len);
	}
	else {
		return false;
	}
}

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(WebHistoryAdhocCommand)

WebHistoryAdhocCommand::WebHistoryAdhocCommand(
	const CString& name, 
	const HISTORY& item
) : 
	AdhocCommandBase(_T(""), item.mDisplayName),
	in(std::make_unique<PImpl>())
{
	in->mName = name;
	in->mHistory = item;
}

WebHistoryAdhocCommand::~WebHistoryAdhocCommand()
{
}

CString WebHistoryAdhocCommand::GetName()
{
	return in->mName + _T(" ") + in->mHistory.mDisplayName;
}

CString WebHistoryAdhocCommand::GetGuideString()
{
	return _T("⏎:ブラウザで開く S-⏎:URLをクリップボードにコピー");
}

CString WebHistoryAdhocCommand::GetTypeDisplayName()
{
	return WebHistoryAdhocCommand::TypeDisplayName((LPCTSTR)in->mHistory.mBrowserName);
}

bool WebHistoryAdhocCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		if (in->mHistory.mBrowserName == _T("Chrome")) {
		*action = new OpenInChromeAction(in->mHistory.mUrl);
		return true;
		}
		else if (in->mHistory.mBrowserName == _T("Edge")) {
		*action = new OpenInEdgeAction(in->mHistory.mUrl);
		return true;
		}
		else {
			return false;
		}
	}
	else if (modifierFlags == Command::MODIFIER_SHIFT) {
		*action = new actions::clipboard::CopyTextAction(in->mHistory.mUrl);
		return true;
	}
	return false;
}


HICON WebHistoryAdhocCommand::GetIcon()
{
	// ブラウザに応じたアイコンを取得
	std::vector<TCHAR> path(MAX_PATH_NTFS);
	in->GetExecutablePath(path.data(), MAX_PATH_NTFS);
	return IconLoader::Get()->LoadIconFromPath(path.data());
}

launcherapp::core::Command*
WebHistoryAdhocCommand::Clone()
{
	return new WebHistoryAdhocCommand(in->mName, in->mHistory);
}

CString WebHistoryAdhocCommand::GetSourceName()
{
	return in->mName;
}

bool WebHistoryAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString WebHistoryAdhocCommand::TypeDisplayName(LPCTSTR browserName)
{
	static CString TEXT_HISTORY((LPCTSTR)IDS_COMMAND_HISTORY);

	CString str;
	str.Format(_T("%s %s"), browserName, (LPCTSTR)TEXT_HISTORY);

	return str;
}


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

