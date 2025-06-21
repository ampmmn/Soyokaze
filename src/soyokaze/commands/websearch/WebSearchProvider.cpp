#include "pch.h"
#include "WebSearchProvider.h"
#include "commands/websearch/WebSearchCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <list>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace websearch {

using CommandRepository = launcherapp::core::CommandRepository;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(WebSearchProvider)


WebSearchProvider::WebSearchProvider()
{
}

WebSearchProvider::~WebSearchProvider()
{
}

CString WebSearchProvider::GetName()
{
	return _T("WebSearchCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString WebSearchProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_COMMANDNAME_WEBSEARCH);
}

// コマンドの種類の説明を示す文字列を取得
CString WebSearchProvider::GetDescription()
{
	static LPCTSTR description = _T("Web検索を定義するコマンドです\n")
		_T("コマンド名とWebの検索条件を登録することにより、\n")
		_T("「コマンド名 (検索キーワード)」の形で任意のWeb検索を行うことができます\n")
		_T("例: ggl→Google検索  tw→Twitter検索");
	return description;
}

// コマンド新規作成ダイアログ
bool WebSearchProvider::NewDialog(CommandParameter* param)
{
	std::unique_ptr<WebSearchCommand> newCmd;
	if (WebSearchCommand::NewDialog(param, newCmd) == false) {
		return false;
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t WebSearchProvider::GetOrder() const
{
	return 140;
}

void WebSearchProvider::OnBeforeLoad()
{
}

bool WebSearchProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<WebSearchCommand> command(new WebSearchCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.release();
	return true;
}


} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

