#include "pch.h"
#include "BuiltinCommandProvider.h"
#include "commands/builtin/BuiltinCommandFactory.h"
#include "commands/builtin/BuiltinCommandBase.h"
#include "commands/core/CommandRepository.h"
#include "hotkey/CommandHotKeyManager.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "resource.h"
#include <map>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace builtin {

using Entry = CommandFile::Entry;


struct BuiltinCommandProvider::PImpl
{
	uint32_t mRefCount{1};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(BuiltinCommandProvider)


BuiltinCommandProvider::BuiltinCommandProvider() : in(std::make_unique<PImpl>())
{
}

BuiltinCommandProvider::~BuiltinCommandProvider()
{
}

// 初回起動の初期化を行う
void BuiltinCommandProvider::OnFirstBoot()
{
	// コマンド登録
	auto cmdRepo = CommandRepository::GetInstance();

	// 初回起動時に組み込みコマンドをリポジトリに登録する
	auto factory = BuiltinCommandFactory::GetInstance();

	std::vector<CString> typeNames;
	factory->EnumTypeName(typeNames);
	for (auto& type : typeNames) {

		Command* cmd = nullptr;
		if (factory->Create(type, nullptr, &cmd) == false) {
			continue;
		}

		cmdRepo->RegisterCommand(cmd);
	}
}


// コマンドの読み込み
void BuiltinCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	ASSERT(cmdFile);

	auto cmdRepo = CommandRepository::GetInstance();

	auto factory = BuiltinCommandFactory::GetInstance();

	std::set<CString> existingTypes;

	int entries = cmdFile->GetEntryCount();
	for (int i = 0; i < entries; ++i) {

		auto entry = cmdFile->GetEntry(i);
		if (cmdFile->IsUsedEntry(entry)) {
			// 既にロード済(使用済)のエントリ
			continue;
		}

		// エントリのコマンド種別が組み込みコマンドのものならインスタンスを生成する
		CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));

		Command* command = nullptr;
		if (factory->Create(typeStr, entry, &command) == false) {
			continue;
		}

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);

		auto cmdName = command->GetName();

		// 生成したコマンドのインスタンスを登録
		cmdRepo->RegisterCommand(command);

		// システムコマンドとして登場した種別を記憶しておく
		existingTypes.insert(typeStr);
	}

	// システムコマンドがなければ作成しておく
	std::vector<CString> typeNames;
	factory->EnumTypeName(typeNames);
	for (auto& type : typeNames) {

		if (existingTypes.count(type) != 0) {
			// 存在するので作成無用
			continue;
		}

		RefPtr<Command> cmd;
		if (factory->Create(type, nullptr, &cmd) == false) {
			continue;
		}
		if (cmdRepo->HasCommand(cmd->GetName())) {
			continue;
		}

		cmdRepo->RegisterCommand(cmd.release());
	}
}

CString BuiltinCommandProvider::GetName()
{
	return _T("BuiltinCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString BuiltinCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_BUILTINCOMMAND);
}

// コマンドの種類の説明を示す文字列を取得
CString BuiltinCommandProvider::GetDescription()
{
	return CString((LPCTSTR)IDS_DESCRIPTION_BUILTINCOMMAND);
}

// コマンド新規作成ダイアログ
bool BuiltinCommandProvider::NewDialog(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// ToDo: 実装
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool BuiltinCommandProvider::IsPrivate() const
{
	return true;
}

// 一時的なコマンドの準備を行うための初期化。初回のQueryAdhocCommand前に呼ばれる。
void BuiltinCommandProvider::PrepareAdhocCommands()
{
	// なにもしない
}

// 一時的なコマンドを必要に応じて提供する
void BuiltinCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
{
	UNREFERENCED_PARAMETER(pattern);
	UNREFERENCED_PARAMETER(comands);

	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t BuiltinCommandProvider::BuiltinCommandProvider::GetOrder() const
{
	return 200;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t BuiltinCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(BuiltinCommandBase::TypeDisplayName());
	return 1;
}

uint32_t BuiltinCommandProvider::BuiltinCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t BuiltinCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

}
}
}

