#include "pch.h"
#include "ShellExecCommandProvider.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "hotkey/CommandHotKeyManager.h"
#include "commands/shellexecute/ShellExecSettingDialog.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace shellexecute {

using CommandRepository = launcherapp::core::CommandRepository;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ShellExecCommandProvider)

IMPLEMENT_LOADFROM(ShellExecCommandProvider, ShellExecCommand)

ShellExecCommandProvider::ShellExecCommandProvider()
{
}

ShellExecCommandProvider::~ShellExecCommandProvider()
{
}

CString ShellExecCommandProvider::GetName()
{
	return _T("ShellExecuteCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString ShellExecCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_NORMALCOMMAND);
}

// コマンドの種類の説明を示す文字列を取得
CString ShellExecCommandProvider::GetDescription()
{
	return CString((LPCTSTR)IDS_DESCRIPTION_NORMALCOMMAND);
}

// コマンド新規作成ダイアログ
bool ShellExecCommandProvider::NewDialog(CommandParameter* param)
{
	ShellExecCommand* newCmd = nullptr;
	if (ShellExecCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd);
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t ShellExecCommandProvider::GetOrder() const
{
	return 100;
}


}
}
}
