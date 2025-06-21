#include "pch.h"
#include "UserCommandProviderBase.h"
#include "commands/core/CommandRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

using CommandRepository = launcherapp::core::CommandRepository;

UserCommandProviderBase::UserCommandProviderBase() : 
	mRefCount(1)
{
}

UserCommandProviderBase::~UserCommandProviderBase()
{
}

// 初回起動の初期化を行う
void UserCommandProviderBase::OnFirstBoot()
{
	// 特に何もしない
}

// コマンドの読み込み
void UserCommandProviderBase::LoadCommands(
	CommandFile* cmdFile
)
{
	ASSERT(cmdFile);

	OnBeforeLoad();

	auto cmdRepo = CommandRepository::GetInstance();

	int entries = cmdFile->GetEntryCount();
	for (int i = 0; i < entries; ++i) {

		auto entry = cmdFile->GetEntry(i);
		if (cmdFile->IsUsedEntry(entry)) {
			// 既にロード済(使用済)のエントリ
			continue;
		}

		RefPtr<Command> command;
		if (LoadFrom( entry, &command) == false) {
			continue;
		}

		// 登録
		cmdRepo->RegisterCommand(command.release());

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool UserCommandProviderBase::IsPrivate() const
{
	return false;
}

// 一時的なコマンドの準備を行うための初期化。初回のQueryAdhocCommand前に呼ばれる。
void UserCommandProviderBase::PrepareAdhocCommands()
{
	// なにもしない。派生クラス側で必要に応じて実装する
}

void UserCommandProviderBase::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
{
	UNREFERENCED_PARAMETER(pattern);
	UNREFERENCED_PARAMETER(comands);

	// 基本は何もしない
}

uint32_t UserCommandProviderBase::AddRef()
{
	return (uint32_t)InterlockedIncrement(&mRefCount);
}

uint32_t UserCommandProviderBase::Release()
{
	auto n = InterlockedDecrement(&mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

void UserCommandProviderBase::OnBeforeLoad()
{
	// ロード開始時の処理。必要に応じて派生クラス側で実装する
}

bool UserCommandProviderBase::LoadFrom(CommandEntryIF* entry, Command** command)
{
	UNREFERENCED_PARAMETER(entry);
	UNREFERENCED_PARAMETER(command);

	// 派生クラス側で実装する必要あり
	ASSERT(0);
	return false;
}

} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

