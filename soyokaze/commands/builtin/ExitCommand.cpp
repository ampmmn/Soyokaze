#include "pch.h"
#include "framework.h"
#include "ExitCommand.h"
#include "SharedHwnd.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString ExitCommand::TYPE(_T("Builtin-Exit"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(ExitCommand)

CString ExitCommand::GetType()
{
	return TYPE;
}

ExitCommand::ExitCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("exit"))
{
	mDescription = _T("【終了】");
}

ExitCommand::~ExitCommand()
{
}

BOOL ExitCommand::Execute(const Parameter& param)
{
	if (AfxGetMainWnd() != nullptr) {
		// AfxGetGetMainWnd()でウインドウがとれる==メインスレッドなので、
		// この場合はたんにPostQuitMessage(0)でOK
		PostQuitMessage(0);
		return TRUE;
	}
	else {
		// 別スレッド経由でコマンドが実行される場合があるので、
		// PostMessageでメインウインドウに配送することにより
		// メインウインドウ側スレッドの処理として終了処理を行う
		SharedHwnd sharedHwnd;
		HWND hwnd = sharedHwnd.GetHwnd();
		if (hwnd) {
			PostMessage(hwnd, WM_APP+8, 0, 0);
		}
		return TRUE;
	}
}


HICON ExitCommand::GetIcon()
{
	return IconLoader::Get()->LoadExitIcon();
}
launcherapp::core::Command* ExitCommand::Clone()
{
	return new ExitCommand();
}

}
}
}
