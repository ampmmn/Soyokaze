#include "pch.h"
#include "SimpleDictProvider.h"
#include "commands/simple_dict/SimpleDictCommand.h"
#include "commands/simple_dict/SimpleDictAdhocCommand.h"
#include "commands/simple_dict/SimpleDictDatabase.h"
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

struct SimpleDictProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
	}

	bool GetParam(const CString& name, SimpleDictParam& param);

	std::vector<SimpleDictCommand*> mCommands;
	std::unique_ptr<SimpleDictDatabase> mDatabase;

	uint32_t mRefCount = 1;
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
	for (auto command : in->mCommands) {
		command->RemoveListener(in->mDatabase.get());
		command->Release();
	}
}

// 初回起動の初期化を行う
void SimpleDictProvider::OnFirstBoot()
{
}

// コマンドの読み込み
void SimpleDictProvider::LoadCommands(CommandFile* cmdFile)
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

		auto command = std::make_unique<SimpleDictCommand>();
		if (command->Load(entry) == false) {
			continue;
		}

		in->mCommands.push_back(command.get());
		command->AddRef();
		command->AddListener(in->mDatabase.get());

		// 登録
		cmdRepo->RegisterCommand(command.release());

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
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
	CString description(_T("簡易的な辞書を定義します。\n"));
	description += _T("Excelファイル内の任意の範囲のデータを簡易的な辞書として利用することができるコマンドです。\n");
	description += _T("(要Excel)");
	return description;
}

// コマンド新規作成ダイアログ
bool SimpleDictProvider::NewDialog(const CommandParameter* param)
{
	SimpleDictCommand* newCmd = nullptr;
	if (SimpleDictCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}
	CommandRepository::GetInstance()->RegisterCommand(newCmd);

	in->mCommands.push_back(newCmd);
	newCmd->AddRef();

	newCmd->AddListener(in->mDatabase.get());

	return true;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool SimpleDictProvider::IsPrivate() const
{
	return false;
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

		commands.push_back(CommandQueryItem(item.mMatchLevel, cmd));
	}
}

// 設定ページを取得する
bool SimpleDictProvider::CreateSettingPages(CWnd* parent, std::vector<SettingPage*>& pages)
{
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t SimpleDictProvider::GetOrder() const
{
	return 2000;
}

uint32_t SimpleDictProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t SimpleDictProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}



} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

