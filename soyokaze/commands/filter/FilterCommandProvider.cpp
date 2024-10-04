#include "pch.h"
#include "FilterCommandProvider.h"
#include "commands/filter/FilterCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace filter {


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(FilterCommandProvider)

FilterCommandProvider::FilterCommandProvider()
{
}

FilterCommandProvider::~FilterCommandProvider()
{
}

CString FilterCommandProvider::GetName()
{
	return _T("FilterCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString FilterCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_FILTERCOMMAND);
}

// コマンドの種類の説明を示す文字列を取得
CString FilterCommandProvider::GetDescription()
{
	return CString((LPCTSTR)IDS_DESCRIPTION_FILTERCOMMAND);
}

// コマンド新規作成ダイアログ
bool FilterCommandProvider::NewDialog(const CommandParameter* param)
{
	FilterCommand* newCmd = nullptr;
	if (FilterCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	constexpr bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd, isReloadHotKey);

	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t FilterCommandProvider::FilterCommandProvider::GetOrder() const
{
	return 400;
}

void FilterCommandProvider::OnBeforeLoad()
{
}

bool FilterCommandProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<FilterCommand> command(new FilterCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.release();

	return true;
}

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

