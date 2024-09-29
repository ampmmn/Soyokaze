#include "pch.h"
#include "SimpleDictProvider.h"
#include "commands/simple_dict/SimpleDictCommand.h"
#include "commands/simple_dict/SimpleDictAdhocCommand.h"
#include "commands/simple_dict/SimpleDictDatabase.h"
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
namespace simple_dict {

using CommandRepository = launcherapp::core::CommandRepository;
using CommandRepositoryListenerIF = launcherapp::core::CommandRepositoryListenerIF;

struct SimpleDictProvider::PImpl : public AppPreferenceListenerIF, public CommandRepositoryListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
	}

	bool GetParam(const CString& name, SimpleDictParam& param);
	void ClearCommands();

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
		SimpleDictCommand* newCmd = nullptr;
		if (SimpleDictCommand::CastFrom(cmd, &newCmd)) {
			mCommands.push_back(newCmd);
		}
	}

	void OnDeleteCommand(Command* command) override
	{
		SimpleDictCommand* cmd = nullptr;
		if (SimpleDictCommand::CastFrom(command, &cmd) == false) {
			return;
		}
		auto it = std::find(mCommands.begin(), mCommands.end(), cmd);
		if (it != mCommands.end()) {
			mCommands.erase(it);
			cmd->RemoveListener(mDatabase.get());
			cmd->Release();
		}
	}
	void OnLancuherActivate() override
	{
	}
	void OnLancuherUnactivate() override
	{
	}


	std::vector<SimpleDictCommand*> mCommands;
	std::unique_ptr<SimpleDictDatabase> mDatabase;
};

bool SimpleDictProvider::PImpl::GetParam(const CString& name, SimpleDictParam& param)
{
	for (auto& cmd : mCommands) {
		if (name != cmd->GetName()) {
			continue;
		}
		param = cmd->GetParam();
		return true;
	}
	return false;
}

void SimpleDictProvider::PImpl::ClearCommands()
{
	for (auto command : mCommands) {
		command->RemoveListener(mDatabase.get());
		command->Release();
	}
	mCommands.clear();
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(SimpleDictProvider)


SimpleDictProvider::SimpleDictProvider() : in(std::make_unique<PImpl>())
{
	in->mDatabase.reset(new SimpleDictDatabase());
}

SimpleDictProvider::~SimpleDictProvider()
{
	in->ClearCommands();
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
bool SimpleDictProvider::NewDialog(const CommandParameter* param)
{
	SimpleDictCommand* newCmd = nullptr;
	if (SimpleDictCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd, isReloadHotKey);

	newCmd->AddListener(in->mDatabase.get());

	return true;
}

// 一時的なコマンドを必要に応じて提供する
void SimpleDictProvider::QueryAdhocCommands(
	Pattern* pattern,
 	launcherapp::CommandQueryItemList& commands
)
{
	std::vector<SimpleDictDatabase::ITEM> items;
	in->mDatabase->Query(pattern, items, 20, 100);
	for (auto& item : items) {
		SimpleDictParam param;
		if (in->GetParam(item.mName, param) == false) {
			continue;
		}

		auto cmd = new SimpleDictAdhocCommand(item.mKey, item.mValue);
		cmd->SetParam(param);

		// 最低でも前方一致扱いにする(先頭のコマンド名は合致しているため)
		int level = item.mMatchLevel;
		if (param.mIsMatchWithoutKeyword == false && level == Pattern::PartialMatch) {
			level = Pattern::FrontMatch;
		}
		commands.Add(CommandQueryItem(level, cmd));
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t SimpleDictProvider::GetOrder() const
{
	return 2000;
}

void SimpleDictProvider::OnBeforeLoad()
{
	in->ClearCommands();
}

bool SimpleDictProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<SimpleDictCommand> command(new SimpleDictCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);

	command->AddListener(in->mDatabase.get());
	*retCommand = command.release();
	return true;
}



} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

