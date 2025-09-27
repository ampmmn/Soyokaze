#include "pch.h"
#include "WebHistoryAdhocCommand.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/common/SubProcess.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "utility/Path.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "framework.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace webhistory {

struct WebHistoryAdhocCommand::PImpl
{
	CString mName;
	HISTORY mHistory;
	CString mProductName;
};

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
	item.mEnv->GetProductName(in->mProductName);
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
	return WebHistoryAdhocCommand::TypeDisplayName(in->mProductName);
}

BOOL WebHistoryAdhocCommand::Execute(Parameter* param)
{
	bool isShiftKeyPressed = GetModifierKeyState(param, MASK_SHIFT) != 0;
	if (isShiftKeyPressed) {
		// URLをクリップボードにコピー
		Clipboard::Copy(in->mHistory.mUrl);
		return TRUE;
	}

	auto& history = in->mHistory;
	// URLをブラウザで開く
	CString path;
	if (history.mEnv->GetInstalledExePath(path) == false) {
		CString msg(_T("Browser executable not found."));
		msg += _T("\n");
		msg += path;
		AfxMessageBox(msg);
		return TRUE;
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	exec.Run(path, history.mUrl, process);

	return TRUE;
}

HICON WebHistoryAdhocCommand::GetIcon()
{
	// ブラウザに応じたアイコンを取得
	CString path;
	in->mHistory.mEnv->GetInstalledExePath(path);
	return IconLoader::Get()->LoadIconFromPath(path);
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
	str.Format(_T("%s %s"), (LPCTSTR)TEXT_HISTORY, browserName);

	return str;
}


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

