#include "pch.h"
#include "EjectVolumeProvider.h"
#include "commands/ejectvolume/EjectVolumeCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "hotkey/CommandHotKeyManager.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace ejectvolume {

struct EjectVolumeProvider::PImpl
{
	ULONG mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(EjectVolumeProvider)


EjectVolumeProvider::EjectVolumeProvider() : in(std::make_unique<PImpl>())
{
	in->mRefCount = 1;
}

EjectVolumeProvider::~EjectVolumeProvider()
{
}

// 初回起動の初期化を行う
void EjectVolumeProvider::OnFirstBoot()
{
	// 特に何もしない
}


// コマンドの読み込み
void EjectVolumeProvider::LoadCommands(
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
		if (typeStr.IsEmpty() == FALSE && typeStr != EjectVolumeCommand::GetType()) {
			continue;
		}

		auto command = std::make_unique<EjectVolumeCommand>();
		if (command->Load(cmdFile, entry) == false) {
			continue;
		}

		// 登録
		cmdRepo->RegisterCommand(command.release());

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
}

CString EjectVolumeProvider::GetName()
{
	return _T("EjectVolumeCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString EjectVolumeProvider::GetDisplayName()
{
	return _T("取り外しコマンド");
}

// コマンドの種類の説明を示す文字列を取得
CString EjectVolumeProvider::GetDescription()
{
	CString str;
	str += _T("リムーバブルメディアの取り外しやCDドライブトレイを開きます");

	return str;
}

// コマンド新規作成ダイアログ
bool EjectVolumeProvider::NewDialog(const CommandParameter* param)
{
	return EjectVolumeCommand::NewDialog(param);
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool EjectVolumeProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void EjectVolumeProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
{
	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t EjectVolumeProvider::EjectVolumeProvider::GetOrder() const
{
	return 3000;
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool EjectVolumeProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	// 必要に応じて実装する
	return true;
}

uint32_t EjectVolumeProvider::EjectVolumeProvider::AddRef()
{
	return InterlockedIncrement(&in->mRefCount);
}

uint32_t EjectVolumeProvider::Release()
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace ejectvolume
} // end of namespace commands
} // end of namespace launcherapp

