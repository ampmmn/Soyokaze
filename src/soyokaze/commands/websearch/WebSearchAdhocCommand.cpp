#include "pch.h"
#include "framework.h"
#include "WebSearchAdhocCommand.h"
#include "commands/websearch/WebSearchCommand.h"
#include "commands/websearch/WebSearchCommandParam.h"
#include "actions/web/OpenURLAction.h"
#include "actions/builtin/CallbackAction.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/websearch/SearchViewerProxy.h" // 専用検索ウインドウへの要求を送る
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
	bool mIsUseSearchViewer{false};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(WebSearchAdhocCommand)

WebSearchAdhocCommand::WebSearchAdhocCommand(
	WebSearchCommand* baseCommand,
 	const CString& displayName,
	const CString& showUrl
) : 
	AdhocCommandBase(displayName, baseCommand->HasRawDescription() ? baseCommand->GetDescription() : showUrl),
	in(std::make_unique<PImpl>())
{
	in->mBaseCommand = baseCommand;
	baseCommand->AddRef();
	in->mURL = showUrl;
	in->mIsUseSearchViewer = baseCommand->IsUseSearchViewer();
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

/**
  検索先の設定と修飾キーに応じたアクションを生成する
  @return アクションを生成した場合はtrue
  @param[in] hotkeyAttr 起動時の修飾キー
  @param[out] action 生成したアクション
*/
bool WebSearchAdhocCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	const bool isDefaultBrowserAction = hotkeyAttr.GetModifiers() == MOD_CONTROL && hotkeyAttr.GetVKCode() == VK_RETURN;
	if (hotkeyAttr.GetModifiers() != 0 && (in->mIsUseSearchViewer == false || isDefaultBrowserAction == false)) {
		return false;
	}

	if (in->mIsUseSearchViewer && isDefaultBrowserAction == false) {
		const CString commandName = in->mBaseCommand->GetName();
		const CString url = in->mURL;
		auto viewerAction = new launcherapp::actions::builtin::CallbackAction(_T("検索を実行"), [commandName, url](Parameter* param, String* errMsg) -> bool {
			auto expandedUrl = url;
			launcherapp::commands::common::ExpandArgumentsAndMacros(expandedUrl, param, launcherapp::commands::common::ArgumentQuoting::None, true);
			if (SearchViewerProxy::GetInstance()->Open(commandName, expandedUrl)) {
				return true;
			}
			if (errMsg) {
				*errMsg = _T("専用検索ウインドウを開けませんでした");
			}
			return false;
		});
		*action = viewerAction;
		return true;
	}

	auto browserAction = new actions::web::OpenURLAction(in->mURL);
	browserAction->SetDisplayName(isDefaultBrowserAction ? _T("既定のWebブラウザで検索") : _T("検索を実行"));
	*action = browserAction;
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
		return GetAction(HOTKEY_ATTR(0, VK_RETURN), action);
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

