#include "pch.h"
#include "framework.h"
#include "commands/builtin/RegistWinCommand.h"
#include "core/CommandRepository.h"
#include "IconLoader.h"
#include "CommandFile.h"
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


struct RegistWinCommand::PImpl
{
	HWND GetTargetWindow(HWND startHwnd)
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
	CString mErrorMsg;
	CString mName;
	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString RegistWinCommand::GetType() { return _T("Builtin-RegisterWindow"); }


RegistWinCommand::RegistWinCommand(LPCTSTR name) : in(std::make_unique<PImpl>())
{
	in->mName = name ? name : _T("registwin");
	in->mRefCount = 1;
}

RegistWinCommand::~RegistWinCommand()
{
}

CString RegistWinCommand::GetName()
{
	return in->mName;
}

CString RegistWinCommand::GetDescription()
{
	return _T("【アクティブウインドウ登録】");
}

CString RegistWinCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_BUILTIN);
	return TEXT_TYPE;
}

BOOL RegistWinCommand::Execute()
{
	in->mErrorMsg.Empty();

	SharedHwnd sharedHwnd;

	HWND hNextWindow =
	 	in->GetTargetWindow(sharedHwnd.GetHwnd());
	if (hNextWindow == NULL) {
		in->mErrorMsg.LoadString(IDS_ERR_GETNEXTWINDOW);
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
		in->mErrorMsg.LoadString(IDS_ERR_QUERYPROCESSINFO);

		CString msg;
		msg.Format(_T(" (PID:%d)"), e.GetPID());
		in->mErrorMsg += msg;

		return FALSE;
	}


}

BOOL RegistWinCommand::Execute(const Parameter& param)
{
	return Execute();
}

CString RegistWinCommand::GetErrorString()
{
	return in->mErrorMsg;
}

HICON RegistWinCommand::GetIcon()
{
	return IconLoader::Get()->LoadRegisterWindowIcon();
}

int RegistWinCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool RegistWinCommand::IsEditable()
{
	return false;
}

int RegistWinCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* RegistWinCommand::Clone()
{
	return new RegistWinCommand();
}

bool RegistWinCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
}

uint32_t RegistWinCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t RegistWinCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

