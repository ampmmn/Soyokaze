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


REGISTER_COMMANDPROVIDER(VolumeCommandProvider)

IMPLEMENT_LOADFROM(VolumeCommandProvider, VolumeCommand)

VolumeCommandProvider::VolumeCommandProvider()
{
}

VolumeCommandProvider::~VolumeCommandProvider()
{
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
bool VolumeCommandProvider::NewDialog(CommandParameter* param)
{
	return VolumeCommand::NewDialog(param);
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t VolumeCommandProvider::VolumeCommandProvider::GetOrder() const
{
	return 2500;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t VolumeCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(VolumeCommand::TypeDisplayName());
	return 1;
}

} // end of namespace volumecontrol
} // end of namespace commands
} // end of namespace launcherapp

