#include "pch.h"
#include "URLDirectoryIndexCommandProvider.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommand.h"
#include "commands/url_directoryindex/DirectoryIndexAdhocCommand.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandParam.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/CommandParameter.h"
#include "hotkey/CommandHotKeyManager.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using CommandRepositoryListenerIF = launcherapp::core::CommandRepositoryListenerIF;

namespace launcherapp {
namespace commands {
namespace url_directoryindex {


struct URLDirectoryIndexCommandProvider::PImpl : public AppPreferenceListenerIF, public CommandRepositoryListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
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
	}
	void OnLancuherUnactivate() override
	{
	}

	std::vector<URLDirectoryIndexCommand*> mCommands;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(URLDirectoryIndexCommandProvider)


URLDirectoryIndexCommandProvider::URLDirectoryIndexCommandProvider() : in(std::make_unique<PImpl>())
{
}

URLDirectoryIndexCommandProvider::~URLDirectoryIndexCommandProvider()
{
}

CString URLDirectoryIndexCommandProvider::GetName()
{
	return _T("URLDirectoryIndexCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString URLDirectoryIndexCommandProvider::GetDisplayName()
{
	return _T("DirectoryIndexコマンド");
}

// コマンドの種類の説明を示す文字列を取得
CString URLDirectoryIndexCommandProvider::GetDescription()
{
	static LPCTSTR description = _T("DirectoryIndexページに表示される項目を候補として表示します");
	return description;
}

// コマンド新規作成ダイアログ
bool URLDirectoryIndexCommandProvider::NewDialog(const CommandParameter* param)
{
	URLDirectoryIndexCommand* newCmd = nullptr;
	if (URLDirectoryIndexCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	constexpr bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd, isReloadHotKey);

	in->mCommands.push_back(newCmd);
	newCmd->AddRef();

	return true;
}

// 一時的なコマンドを必要に応じて提供する
void URLDirectoryIndexCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
	DirectoryIndexQueryResult results;
	for (auto& cmd : in->mCommands) {

		results.clear();
		cmd->Query(pattern, results);

		CommandParam cmdParam;
		cmd->GetParam(cmdParam);

		for (auto& result : results) {
			auto adhocCmd = new DirectoryIndexAdhocCommand(cmd, result);
			commands.push_back(CommandQueryItem(result.mMatchLevel, adhocCmd));
		}
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t URLDirectoryIndexCommandProvider::URLDirectoryIndexCommandProvider::GetOrder() const
{
	return 400;
}

bool URLDirectoryIndexCommandProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<URLDirectoryIndexCommand> command(new URLDirectoryIndexCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.get();

	command->AddRef();
	in->mCommands.push_back(command.release());

	return true;
}


} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

