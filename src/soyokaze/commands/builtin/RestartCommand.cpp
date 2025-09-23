#include "pch.h"
#include "framework.h"
#include "RestartCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/SubProcess.h"
#include "actions/core/ActionParameter.h"
#include "icon/IconLoader.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using SubProcess = launcherapp::commands::common::SubProcess;

namespace launcherapp {
namespace commands {
namespace builtin {


CString RestartCommand::TYPE(_T("Builtin-Restart"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(RestartCommand)

CString RestartCommand::GetType()
{
	return TYPE;
}

RestartCommand::RestartCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("restart"))
{
	mDescription = _T("【アプリの再起動】");
	mCanSetConfirm = true;
	mCanDisable = true;
	mIsEnable = false;
}

RestartCommand::RestartCommand(const RestartCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

RestartCommand::~RestartCommand()
{
}

BOOL RestartCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	if (mIsConfirmBeforeRun) {
		if (AfxMessageBox(_T("アプリを再起動しますか?"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) {
			return TRUE;
		}
	}

	// launcher_proxy.exe経由でアプリを再起動する
	Path proxyPath(Path::MODULEFILEDIR);
	proxyPath.Append(_T("launcher_proxy.exe"));
	if (proxyPath.FileExists() == false) {
		return FALSE;
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(ParameterBuilder::EmptyParam());
	exec.SetShowType(SW_HIDE);

	// 管理者権限で実行している場合は、launcher_proxyも管理者権限で実行する
	// (でないと、再起動に必要な情報をlauncher_proxy側で取得できないため)
	if (SubProcess::IsRunningAsAdmin()) {
		exec.SetRunAsAdmin();
	}

	exec.Run((LPCTSTR)proxyPath, _T("restart"), process);

	return TRUE;
}

HICON RestartCommand::GetIcon()
{
	return IconLoader::Get()->LoadReloadIcon();
}


launcherapp::core::Command* RestartCommand::Clone()
{
	return new RestartCommand(*this);
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

