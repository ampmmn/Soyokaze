#include "pch.h"
#include "FilterCommandProvider.h"
#include "commands/filter/FilterCommand.h"
#include "commands/filter/FilterAdhocCommand.h"
#include "commands/filter/FilterCommandParam.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/CommandParameter.h"
#include "hotkey/CommandHotKeyManager.h"
#include "FilterEditDialog.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "commands/core/CommandFile.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "resource.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using CommandRepositoryListenerIF = launcherapp::core::CommandRepositoryListenerIF;

namespace launcherapp {
namespace commands {
namespace filter {


struct FilterCommandProvider::PImpl : public AppPreferenceListenerIF, public CommandRepositoryListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		ClearCommands();
	}

	void ClearCommands() {
		for (auto command : mCommands) {
			command->Release();
		}
		mCommands.clear();
	}

// AppPreferenceListenerIF
	void OnAppFirstBoot() override
	{
		OnAppNormalBoot();
	}
	void OnAppNormalBoot() override
	{
		auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
		cmdRepo->RegisterListener(this);
	}

	void OnAppPreferenceUpdated() override
	{
	}
	void OnAppExit() override
	{
		auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
		cmdRepo->UnregisterListener(this);
	}


// CommandRepositoryListenerIF
	void OnNewCommand(launcherapp::core::Command* cmd) override
	{
		FilterCommand* newCmd = nullptr;
		if (FilterCommand::CastFrom(cmd, &newCmd)) {
			mCommands.push_back(newCmd);
		}
	}

	void OnDeleteCommand(Command* command) override
	{
		auto it = std::find(mCommands.begin(), mCommands.end(), command);
		if (it != mCommands.end()) {
			mCommands.erase(it);
			command->Release();
		}
	}
	void OnLancuherActivate() override
	{
		for (auto cmd : mCommands) {
			cmd->ClearCache();
		}
	}
	void OnLancuherUnactivate() override
	{
	}

	std::vector<FilterCommand*> mCommands;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(FilterCommandProvider)

FilterCommandProvider::FilterCommandProvider() : in(std::make_unique<PImpl>())
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

// 一時的なコマンドを必要に応じて提供する
void FilterCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
	std::vector<FilterResult> results;
	for (auto& cmd : in->mCommands) {

		results.clear();
		cmd->Query(pattern, results);

		CommandParam cmdParam;
		cmd->GetParam(cmdParam);

		for (auto& result : results) {
			auto adhocCmd = new FilterAdhocCommand(cmdParam, result);
			commands.Add(CommandQueryItem(result.mMatchLevel, adhocCmd));
		}
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t FilterCommandProvider::FilterCommandProvider::GetOrder() const
{
	return 400;
}

void FilterCommandProvider::OnBeforeLoad()
{
	in->ClearCommands();
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

