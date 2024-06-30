#include "pch.h"
#include "VolumeCommandProvider.h"
#include "commands/volumecontrol/VolumeCommand.h"
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
namespace volumecontrol {


struct VolumeCommandProvider::PImpl
{
	ULONG mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(VolumeCommandProvider)


VolumeCommandProvider::VolumeCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mRefCount = 1;
}

VolumeCommandProvider::~VolumeCommandProvider()
{
}

// 初回起動の初期化を行う
void VolumeCommandProvider::OnFirstBoot()
{
	// 特に何もしない
}


// コマンドの読み込み
void VolumeCommandProvider::LoadCommands(
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

		auto command = std::make_unique<VolumeCommand>();
		if (command->Load(entry) == false) {
			continue;
		}

		// 登録
		bool isReloadHotKey = false;
		cmdRepo->RegisterCommand(command.release(), isReloadHotKey);

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
}

CString VolumeCommandProvider::GetName()
{
	return _T("VolumeCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString VolumeCommandProvider::GetDisplayName()
{
	return _T("音量調節");
}

// コマンドの種類の説明を示す文字列を取得
CString VolumeCommandProvider::GetDescription()
{
	CString str;
	str += _T("コマンド実行時のサウンドデバイスの音量調節をします\n");
	str += _T("音量変更とミュート切り替えができます");

	return str;
}

// コマンド新規作成ダイアログ
bool VolumeCommandProvider::NewDialog(const CommandParameter* param)
{
	return VolumeCommand::NewDialog(param);
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool VolumeCommandProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void VolumeCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands)
{
	// このCommandProviderは一時的なコマンドを持たない
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t VolumeCommandProvider::VolumeCommandProvider::GetOrder() const
{
	return 2500;
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool VolumeCommandProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	// 必要に応じて実装する
	return true;
}

uint32_t VolumeCommandProvider::VolumeCommandProvider::AddRef()
{
	return InterlockedIncrement(&in->mRefCount);
}

uint32_t VolumeCommandProvider::Release()
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace volumecontrol
} // end of namespace commands
} // end of namespace launcherapp

