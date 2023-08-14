#include "pch.h"
#include "BuiltinCommandProvider.h"
#include "commands/builtin/NewCommand.h"
#include "commands/builtin/ReloadCommand.h"
#include "commands/builtin/EditCommand.h"
#include "commands/builtin/ExitCommand.h"
#include "commands/builtin/VersionCommand.h"
#include "commands/builtin/UserDirCommand.h"
#include "commands/builtin/MainDirCommand.h"
#include "commands/builtin/SettingCommand.h"
#include "commands/builtin/ManagerCommand.h"
#include "commands/builtin/RegistWinCommand.h"
#include "commands/builtin/ChangeDirectoryCommand.h"
#include "commands/builtin/DeleteCommand.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "core/CommandHotKeyManager.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "CommandHotKeyMappings.h"
#include "resource.h"
#include <map>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = soyokaze::core::CommandRepository;

namespace soyokaze {
namespace commands {
namespace builtin {


struct BuiltinCommandProvider::PImpl
{
	uint32_t mRefCount;
	std::map<CString, std::function<soyokaze::core::Command*(LPCTSTR)> > mFactoryMap;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(BuiltinCommandProvider)


BuiltinCommandProvider::BuiltinCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mRefCount = 1;

	in->mFactoryMap[NewCommand::GetType()] = [](LPCTSTR name) { return new NewCommand(name); };
	in->mFactoryMap[EditCommand::GetType()] = [](LPCTSTR name) { return new EditCommand(name); };
	in->mFactoryMap[ReloadCommand::GetType()] = [](LPCTSTR name) { return new ReloadCommand(name); };
	in->mFactoryMap[ManagerCommand::GetType()] = [](LPCTSTR name) { return new ManagerCommand(name); };
	in->mFactoryMap[ExitCommand::GetType()] = [](LPCTSTR name) { return new ExitCommand(name); };
	in->mFactoryMap[VersionCommand::GetType()] = [](LPCTSTR name) { return new VersionCommand(name); };
	in->mFactoryMap[UserDirCommand::GetType()] = [](LPCTSTR name) { return new UserDirCommand(name); };
	in->mFactoryMap[MainDirCommand::GetType()] = [](LPCTSTR name) { return new MainDirCommand(name); };
	in->mFactoryMap[SettingCommand::GetType()] = [](LPCTSTR name) { return new SettingCommand(name); };
	in->mFactoryMap[RegistWinCommand::GetType()] = [](LPCTSTR name) { return new RegistWinCommand(name); };
	in->mFactoryMap[ChangeDirectoryCommand::GetType()] = [](LPCTSTR name) { return new ChangeDirectoryCommand(name); };
	in->mFactoryMap[DeleteCommand::GetType()] = [](LPCTSTR name) { return new DeleteCommand(name); };

}

BuiltinCommandProvider::~BuiltinCommandProvider()
{
}

// 初回起動の初期化を行う
void BuiltinCommandProvider::OnFirstBoot()
{
	// コマンド登録
	auto cmdRepo = CommandRepository::GetInstance();

	for (auto entry : in->mFactoryMap) {
		cmdRepo->RegisterCommand(entry.second(nullptr));
	}
}


// コマンドの読み込み
void BuiltinCommandProvider::LoadCommands(
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

		CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));
		auto itFind = in->mFactoryMap.find(typeStr);
		if (itFind == in->mFactoryMap.end()) {
			continue;
		}

		CString name = cmdFile->GetName(entry);

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);

		auto command = itFind->second(name);

		// 登録
		cmdRepo->RegisterCommand(command);
	}

	// システムコマンドがなければ作成しておく
	for (auto& elem : in->mFactoryMap) {
		auto cmd = elem.second(nullptr);

		if (cmdRepo->HasCommand(cmd->GetName())) {
			cmd->Release();
			continue;
		}
		cmdRepo->RegisterCommand(cmd);
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
bool BuiltinCommandProvider::NewDialog(const CommandParameter* param)
{
	// ToDo: 実装
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool BuiltinCommandProvider::IsPrivate() const
{
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void BuiltinCommandProvider::QueryAdhocCommands(Pattern* pattern, std::vector<CommandQueryItem>& comands)
{
	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t BuiltinCommandProvider::BuiltinCommandProvider::GetOrder() const
{
	return 200;
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

