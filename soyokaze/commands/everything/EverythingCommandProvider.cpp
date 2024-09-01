#include "pch.h"
#include "EverythingCommandProvider.h"
#include "commands/everything/EverythingCommand.h"
#include "commands/everything/EverythingAdhocCommand.h"
#include "commands/everything/EverythingResult.h"
#include "commands/everything/AppSettingEverythingPage.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandFile.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace everything {

using CommandRepository = launcherapp::core::CommandRepository;
using CommandRepositoryListenerIF = launcherapp::core::CommandRepositoryListenerIF;

struct EverythingCommandProvider::PImpl : public AppPreferenceListenerIF, public CommandRepositoryListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		ClearCommands();
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
	void OnDeleteCommand(Command* command) override
	{
		auto it = std::find(mCommands.begin(), mCommands.end(), command);
		if (it != mCommands.end()) {
			mCommands.erase(it);
			command->Release();
		}
	}
	void OnLancuherActivate() override {}
	void OnLancuherUnactivate() override {}

	void ClearCommands()
	{
		for (auto command : mCommands) {
			command->Release();
		}
		mCommands.clear();
	}

	std::vector<EverythingCommand*> mCommands;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(EverythingCommandProvider)


EverythingCommandProvider::EverythingCommandProvider() : in(std::make_unique<PImpl>())
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
bool EverythingCommandProvider::NewDialog(const CommandParameter* param)
{
	EverythingCommand* newCmd = nullptr;
	if (EverythingCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	constexpr bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd, isReloadHotKey);

	in->mCommands.push_back(newCmd);
	newCmd->AddRef();

	return true;
}

// 一時的なコマンドを必要に応じて提供する
void EverythingCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	launcherapp::CommandQueryItemList& commands
)
{
	std::vector<EverythingResult> results;
	for (auto& cmd : in->mCommands) {

		results.clear();
		cmd->Query(pattern, results);

		for (auto& result : results) {

			auto adhocCmd = new EverythingAdhocCommand(cmd->GetParam(), result);
			commands.push_back(CommandQueryItem(result.mMatchLevel, adhocCmd));
		}
	}
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
	in->ClearCommands();
}

bool EverythingCommandProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<EverythingCommand> command(new EverythingCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.get();

	command->AddRef();
	in->mCommands.push_back(command.release());

	return true;
}

} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

