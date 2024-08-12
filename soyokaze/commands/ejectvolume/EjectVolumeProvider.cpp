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

REGISTER_COMMANDPROVIDER(EjectVolumeProvider)

IMPLEMENT_LOADFROM(EjectVolumeProvider, EjectVolumeCommand)

EjectVolumeProvider::EjectVolumeProvider()
{
}

EjectVolumeProvider::~EjectVolumeProvider()
{
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

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t EjectVolumeProvider::EjectVolumeProvider::GetOrder() const
{
	return 3000;
}

} // end of namespace ejectvolume
} // end of namespace commands
} // end of namespace launcherapp

