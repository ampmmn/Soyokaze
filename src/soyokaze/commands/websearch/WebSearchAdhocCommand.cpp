#include "pch.h"
#include "framework.h"
#include "WebSearchAdhocCommand.h"
#include "commands/websearch/WebSearchCommand.h"
#include "commands/websearch/WebSearchCommandParam.h"
#include "actions/web/OpenURLAction.h"
#include "commands/core/CommandRepository.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace websearch {

struct WebSearchAdhocCommand::PImpl
{
	WebSearchCommand* mBaseCommand{nullptr};
	CString mURL;
	CString mErrorMsg;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(WebSearchAdhocCommand)

WebSearchAdhocCommand::WebSearchAdhocCommand(
	WebSearchCommand* baseCommand,
 	const CString& displayName,
 	const CString& showUrl
) : 
	AdhocCommandBase(displayName, baseCommand->GetDescription()),
	in(std::make_unique<PImpl>())
{
	in->mBaseCommand = baseCommand;
	baseCommand->AddRef();
	in->mURL = showUrl;
}

WebSearchAdhocCommand::~WebSearchAdhocCommand()
{
	if (in->mBaseCommand) {
		in->mBaseCommand->Release();
	}
}

CString WebSearchAdhocCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMANDNAME_WEBSEARCH);
	return TEXT_TYPE;
}

bool WebSearchAdhocCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}
	auto a = new actions::web::OpenURLAction(in->mURL);
	a->SetDisplayName(_T("検索を実行"));
	*action = a;
	return true;
}

HICON WebSearchAdhocCommand::GetIcon()
{
	return in->mBaseCommand->GetIcon();
}

launcherapp::core::Command*
WebSearchAdhocCommand::Clone()
{
	return new WebSearchAdhocCommand(in->mBaseCommand, GetName(), in->mURL);
}

// メニューの項目数を取得する
int WebSearchAdhocCommand::GetMenuItemCount()
{
	return 1;
}

// メニューの表示名を取得する
bool WebSearchAdhocCommand::GetMenuItem(int index, Action** action)
{
	if (index == 0) {
		auto a = new actions::web::OpenURLAction(in->mURL);
		a->SetDisplayName(_T("検索を実行"));
		*action = a;
		return true;
	}
	return false;
}

CString WebSearchAdhocCommand::GetSourceName()
{
	return in->mBaseCommand->GetName();
}

bool WebSearchAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	else if (ifid == IFID_EXTRACANDIDATE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidate*)this;
		return true;
	}
	return false;
}

} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

