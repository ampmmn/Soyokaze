#include "pch.h"
#include "WebHistoryProvider.h"
#include "commands/webhistory/WebHistoryCommand.h"
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
namespace webhistory {

using CommandRepository = launcherapp::core::CommandRepository;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(WebHistoryProvider)


WebHistoryProvider::WebHistoryProvider()
{
}

WebHistoryProvider::~WebHistoryProvider()
{
}

CString WebHistoryProvider::GetName()
{
	return _T("WebHistoryCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString WebHistoryProvider::GetDisplayName()
{
	return _T("ブラウザ履歴コマンド");
}

// コマンドの種類の説明を示す文字列を取得
CString WebHistoryProvider::GetDescription()
{
	static LPCTSTR description = _T("ブラウザ(Edge,Chrome)で閲覧したページを検索するコマンドです\n")
		_T("「コマンド名 (キーワード)」でキーワードを含む過去に閲覧したページを候補として表示します\n")
		_T("検索条件をプリセットできるので、特定の話題に関する履歴だけを抽出する使い方ができます\n")
		_T("");

	return description;
}

// コマンド新規作成ダイアログ
bool WebHistoryProvider::NewDialog(CommandParameter* param)
{
	std::unique_ptr<WebHistoryCommand> newCmd;
	if (WebHistoryCommand::NewDialog(param, newCmd) == false) {
		return false;
	}
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t WebHistoryProvider::GetOrder() const
{
	return 145;
}

void WebHistoryProvider::OnBeforeLoad()
{
}

bool WebHistoryProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<WebHistoryCommand> command(new WebHistoryCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.release();

	return true;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t WebHistoryProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(WebHistoryCommand::TypeDisplayName());
	return 1;
}


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

