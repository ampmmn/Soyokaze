#include "pch.h"
#include "RoutineCommandProviderBase.h"
#include "commands/core/CommandRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

using CommandRepository = launcherapp::core::CommandRepository;

RoutineCommandProviderBase::RoutineCommandProviderBase() : 
	mRefCount(1)
{
}

RoutineCommandProviderBase::~RoutineCommandProviderBase()
{
}

// 初回起動の初期化を行う
void RoutineCommandProviderBase::OnFirstBoot()
{
	// 特に何もしない
}

// コマンドの読み込み
void RoutineCommandProviderBase::LoadCommands(
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
bool RoutineCommandProviderBase::IsPrivate() const
{
	return false;
}

void RoutineCommandProviderBase::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
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
bool RoutineCommandProviderBase::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(pages);

	// 必要に応じて実装する
	return true;
}


uint32_t RoutineCommandProviderBase::AddRef()
{
	return (uint32_t)InterlockedIncrement(&mRefCount);
}

uint32_t RoutineCommandProviderBase::Release()
{
	auto n = InterlockedDecrement(&mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

bool RoutineCommandProviderBase::LoadFrom(CommandEntryIF* entry, Command** command)
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

