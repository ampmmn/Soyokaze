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
		for (auto cmd : mCommands) {
			cmd->ClearCache();
		}
	}
	void OnLancuherUnactivate() override
	{
	}

	std::vector<FilterCommand*> mCommands;
	uint32_t mRefCount = 1;
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

// 初回起動の初期化を行う
void FilterCommandProvider::OnFirstBoot()
{
	// 特に何もしない
}


// コマンドの読み込み
void FilterCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	ASSERT(cmdFile);

	auto cmdRepo = CommandRepository::GetInstance();

	int entries = cmdFile->GetEntryCount();
	for (int i = 0; i < entries; ++i) {

		auto entry = cmdFile->GetEntry(i);
		if (cmdFile->IsUsedEntry(entry)) {
			// 既にロード済(使用済)のエントリ
			continue;
		}

		FilterCommand* command = nullptr;
		if (FilterCommand::LoadFrom(cmdFile, entry, &command) == false) {
			if (command) {
				command->Release();
			}
			continue;
		}

		// 登録
		constexpr bool isReloadHotKey = false;
		cmdRepo->RegisterCommand(command, isReloadHotKey);

		in->mCommands.push_back(command);
		command->AddRef();

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
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

	in->mCommands.push_back(newCmd);
	newCmd->AddRef();

	return true;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool FilterCommandProvider::IsPrivate() const
{
	return false;
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
			commands.push_back(CommandQueryItem(result.mMatchLevel, adhocCmd));
		}
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t FilterCommandProvider::FilterCommandProvider::GetOrder() const
{
	return 400;
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool FilterCommandProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(pages);

	// 必要に応じて実装する
	return true;
}

uint32_t FilterCommandProvider::FilterCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t FilterCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

