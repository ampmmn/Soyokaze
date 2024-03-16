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
#include "commands/builtin/EmptyRecycleBinCommand.h"
#include "commands/builtin/LogOffCommand.h"
#include "commands/builtin/ShutdownCommand.h"
#include "commands/builtin/RebootCommand.h"
#include "commands/builtin/LockScreenCommand.h"
#include "commands/builtin/StandbyCommand.h"
#include "commands/builtin/SuspendCommand.h"
#include "commands/builtin/AfxChangeDirectoryCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
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

using CommandRepository = soyokaze::core::CommandRepository;

namespace soyokaze {
namespace commands {
namespace builtin {


struct BuiltinCommandProvider::PImpl
{
	void InitMap()
	{
		if (mIsFirstCall == false) {
			return;
		}

		mFactoryMap[NewCommand::TYPE] = [](LPCTSTR name) { return new NewCommand(name); };
		mFactoryMap[EditCommand::TYPE] = [](LPCTSTR name) { return new EditCommand(name); };
		mFactoryMap[ReloadCommand::TYPE] = [](LPCTSTR name) { return new ReloadCommand(name); };
		mFactoryMap[ManagerCommand::TYPE] = [](LPCTSTR name) { return new ManagerCommand(name); };
		mFactoryMap[ExitCommand::TYPE] = [](LPCTSTR name) { return new ExitCommand(name); };
		mFactoryMap[VersionCommand::TYPE] = [](LPCTSTR name) { return new VersionCommand(name); };
		mFactoryMap[UserDirCommand::TYPE] = [](LPCTSTR name) { return new UserDirCommand(name); };
		mFactoryMap[MainDirCommand::TYPE] = [](LPCTSTR name) { return new MainDirCommand(name); };
		mFactoryMap[SettingCommand::TYPE] = [](LPCTSTR name) { return new SettingCommand(name); };
		mFactoryMap[RegistWinCommand::TYPE] = [](LPCTSTR name) { return new RegistWinCommand(name); };
		mFactoryMap[ChangeDirectoryCommand::TYPE] = [](LPCTSTR name) { return new ChangeDirectoryCommand(name); };
		mFactoryMap[DeleteCommand::TYPE] = [](LPCTSTR name) { return new DeleteCommand(name); };
		mFactoryMap[EmptyRecycleBinCommand::TYPE] = [](LPCTSTR name) { return new EmptyRecycleBinCommand(name); };
		mFactoryMap[LogOffCommand::TYPE] = [](LPCTSTR name) { return new LogOffCommand(name); };
		mFactoryMap[ShutdownCommand::TYPE] = [](LPCTSTR name) { return new ShutdownCommand(name); };
		mFactoryMap[RebootCommand::TYPE] = [](LPCTSTR name) { return new RebootCommand(name); };
		mFactoryMap[LockScreenCommand::TYPE] = [](LPCTSTR name) { return new LockScreenCommand(name); };
		mFactoryMap[StandbyCommand::TYPE] = [](LPCTSTR name) { return new StandbyCommand(name); };
		mFactoryMap[SuspendCommand::TYPE] = [](LPCTSTR name) { return new SuspendCommand(name); };
		mFactoryMap[AfxChangeDirectoryCommand::TYPE] = [](LPCTSTR name) { return new AfxChangeDirectoryCommand(name); };
		mIsFirstCall = false;
	}

	uint32_t mRefCount = 1;
	bool mIsFirstCall = true;
	std::map<CString, std::function<soyokaze::core::Command*(LPCTSTR)> > mFactoryMap;
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
	in->InitMap();

	// コマンド登録
	auto cmdRepo = CommandRepository::GetInstance();

	for (auto& entry : in->mFactoryMap) {
		cmdRepo->RegisterCommand(entry.second(nullptr));
	}
}


// コマンドの読み込み
void BuiltinCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	ASSERT(cmdFile);

	in->InitMap();

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

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool BuiltinCommandProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	// 必要に応じて実装する
	return true;
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

