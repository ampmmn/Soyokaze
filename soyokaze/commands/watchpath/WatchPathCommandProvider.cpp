#include "pch.h"
#include "WatchPathCommandProvider.h"
#include "commands/watchpath/WatchPathCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace watchpath {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(WatchPathCommandProvider)

IMPLEMENT_LOADFROM(WatchPathCommandProvider, WatchPathCommand)

WatchPathCommandProvider::WatchPathCommandProvider()
{
}

WatchPathCommandProvider::~WatchPathCommandProvider()
{
}

CString WatchPathCommandProvider::GetName()
{
	return _T("WatchPathCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString WatchPathCommandProvider::GetDisplayName()
{
	return CString(_T("フォルダ更新検知"));
}

// コマンドの種類の説明を示す文字列を取得
CString WatchPathCommandProvider::GetDescription()
{
	return CString(_T("【試作】フォルダ内の更新を検知するコマンドです。"));
}

// コマンド新規作成ダイアログ
bool WatchPathCommandProvider::NewDialog(const CommandParameter* param)
{
	return WatchPathCommand::NewDialog(param);
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t WatchPathCommandProvider::WatchPathCommandProvider::GetOrder() const
{
	return 2100;
}

}
}
}
