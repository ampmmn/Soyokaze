#include "pch.h"
#include "EverythingCommandProvider.h"
#include "commands/everything/EverythingCommand.h"
#include "commands/everything/AppSettingEverythingPage.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace everything {

using CommandRepository = launcherapp::core::CommandRepository;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(EverythingCommandProvider)


EverythingCommandProvider::EverythingCommandProvider()
{
}

EverythingCommandProvider::~EverythingCommandProvider()
{
}

CString EverythingCommandProvider::GetName()
{
	return _T("EverythingCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString EverythingCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)_T("Everything検索"));
}

// コマンドの種類の説明を示す文字列を取得
CString EverythingCommandProvider::GetDescription()
{
	CString description(_T("Everything検索を定義します。\n"));
	description += _T("条件をプリセットした状態でEverything検索を行い、検索結果を得ることができます。\n");
	description += _T("(Everythingを起動しておく必要があります)");
	return description;
}

// コマンド新規作成ダイアログ
bool EverythingCommandProvider::NewDialog(CommandParameter* param)
{
	EverythingCommand* newCmd = nullptr;
	if (EverythingCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	constexpr bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd, isReloadHotKey);

	return true;
}

// 設定ページを取得する
bool EverythingCommandProvider::CreateSettingPages(CWnd* parent, std::vector<SettingPage*>& pages)
{
	pages.push_back(new AppSettingEverythingPage(parent));
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t EverythingCommandProvider::GetOrder() const
{
	return 2050;
}

void EverythingCommandProvider::OnBeforeLoad()
{
}

bool EverythingCommandProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<EverythingCommand> command(new EverythingCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.release();
	return true;
}

} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

