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
	void OnLancuherActivate() override {}
	void OnLancuherUnactivate() override {}

	std::vector<EverythingCommand*> mCommands;
	uint32_t mRefCount = 1;
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

// 初回起動の初期化を行う
void EverythingCommandProvider::OnFirstBoot()
{
}

// コマンドの読み込み
void EverythingCommandProvider::LoadCommands(CommandFile* cmdFile)
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

		EverythingCommand* command = nullptr;
		if (EverythingCommand::LoadFrom(cmdFile, entry, &command) == false) {
			if (command) {
				command->Release();
			}
			continue;
		}

		// 登録
		cmdRepo->RegisterCommand(command);

		in->mCommands.push_back(command);
		command->AddRef();

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
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
	CommandRepository::GetInstance()->RegisterCommand(newCmd);

	in->mCommands.push_back(newCmd);
	newCmd->AddRef();

	return true;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool EverythingCommandProvider::IsPrivate() const
{
	return false;
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

uint32_t EverythingCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t EverythingCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}



} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

