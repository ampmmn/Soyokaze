#include "pch.h"
#include "framework.h"
#include "commands/builtin/RegistWinCommand.h"
#include "core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "utility/ProcessPath.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {


static HWND GetTargetWindow(HWND startHwnd)
{
	// 自分自身を非表示にした後に最前面になるウインドウのハンドルを取得
	ShowWindow(startHwnd, SW_HIDE);
	// Note: 表示状態を変えてしまうが、現状はこの後の処理で
	//       startHwndのウインドウを非表示にしているのでよしとする
	HWND currentHwnd = GetForegroundWindow();

	DWORD currentPid = GetCurrentProcessId();
	for(HWND hwnd = currentHwnd;;hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {
		if (hwnd == NULL) {
			return NULL;
		}

		// 自プロセスのウインドウは除外
		DWORD pid;
		GetWindowThreadProcessId(hwnd, &pid);
		if (pid == currentPid) {
			continue;
		 
		}
		// 非表示のウインドウも除外
		if (IsWindowVisible(hwnd) == FALSE) {
			continue;
		}
		return hwnd;
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString RegistWinCommand::TYPE(_T("Builtin-RegisterWindow"));

CString RegistWinCommand::GetType()
{
	return TYPE;
}

RegistWinCommand::RegistWinCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("registwin"))
{
	mDescription = _T("【アクティブウインドウ登録】");
}

RegistWinCommand::~RegistWinCommand()
{
}

BOOL RegistWinCommand::Execute(const Parameter& param)
{
	mError.Empty();

	SharedHwnd sharedHwnd;

	HWND hNextWindow =
	 	GetTargetWindow(sharedHwnd.GetHwnd());
	if (hNextWindow == NULL) {
		mError.LoadString(IDS_ERR_GETNEXTWINDOW);
		return FALSE;
	}

	ProcessPath processPath(hNextWindow);

	try {
		soyokaze::core::CommandParameter param;
		param.SetNamedParamString(_T("TYPE"), _T("ShellExecuteCommand"));
		param.SetNamedParamString(_T("COMMAND"), processPath.GetProcessName());
		param.SetNamedParamString(_T("PATH"), processPath.GetProcessPath());

		// ウインドウタイトルを説明として使う
		param.SetNamedParamString(_T("DESCRIPTION"), processPath.GetCaption());
		param.SetNamedParamString(_T("ARGUMENT"), processPath.GetCommandLine());

		auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
		cmdRepoPtr->NewCommandDialog(&param);
		return TRUE;
	}
	catch(ProcessPath::Exception& e) {
		mError.LoadString(IDS_ERR_QUERYPROCESSINFO);

		CString msg;
		msg.Format(_T(" (PID:%d)"), e.GetPID());
		mError += msg;

		return FALSE;
	}
}

HICON RegistWinCommand::GetIcon()
{
	return IconLoader::Get()->LoadRegisterWindowIcon();
}

soyokaze::core::Command* RegistWinCommand::Clone()
{
	return new RegistWinCommand();
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

