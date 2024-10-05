#include "pch.h"
#include "SimpleDictProvider.h"
#include "commands/simple_dict/SimpleDictCommand.h"
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
namespace simple_dict {

using CommandRepository = launcherapp::core::CommandRepository;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(SimpleDictProvider)

SimpleDictProvider::SimpleDictProvider()
{
}

SimpleDictProvider::~SimpleDictProvider()
{
}

CString SimpleDictProvider::GetName()
{
	return _T("SimpleDictCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString SimpleDictProvider::GetDisplayName()
{
	return CString((LPCTSTR)_T("簡易辞書コマンド"));
}

// コマンドの種類の説明を示す文字列を取得
CString SimpleDictProvider::GetDescription()
{
	CString description;
	description += _T("Excelファイル内の任意の範囲をKey-Value型のデータベースと見立てるコマンドです\n");
	description += _T("利用例として、キーや値で絞り込んで、選択したものをクリップボードにコピーする、\nといった辞書的な使い方ができます。\n");
	description += _T("(Excelが必要です)");
	return description;
}

// コマンド新規作成ダイアログ
bool SimpleDictProvider::NewDialog(CommandParameter* param)
{
	SimpleDictCommand* newCmd = nullptr;
	if (SimpleDictCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd, isReloadHotKey);
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t SimpleDictProvider::GetOrder() const
{
	return 2000;
}

void SimpleDictProvider::OnBeforeLoad()
{
}

bool SimpleDictProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<SimpleDictCommand> command(new SimpleDictCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);

	*retCommand = command.release();
	return true;
}



} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

