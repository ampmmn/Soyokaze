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

	auto cmdRepo = CommandRepository::GetInstance();

	int entries = cmdFile->GetEntryCount();
	for (int i = 0; i < entries; ++i) {

		auto entry = cmdFile->GetEntry(i);
		if (cmdFile->IsUsedEntry(entry)) {
			// 既にロード済(使用済)のエントリ
			continue;
		}

		Command* command = nullptr;
		if (LoadFrom( entry, &command) == false) {
			if (command) {
				command->Release();
			}
			continue;
		}

		// 登録
		constexpr bool isReloadHotKey = false;
		cmdRepo->RegisterCommand(command, isReloadHotKey);

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool UserCommandProviderBase::IsPrivate() const
{
	return false;
}

void UserCommandProviderBase::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
{
	UNREFERENCED_PARAMETER(pattern);
	UNREFERENCED_PARAMETER(comands);

	// 基本は何もしない
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool UserCommandProviderBase::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(pages);

	// 必要に応じて実装する
	return true;
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

