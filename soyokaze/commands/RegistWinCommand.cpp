#include "pch.h"
#include "framework.h"
#include "commands/RegistWinCommand.h"
#include "CommandRepository.h"
#include "IconLoader.h"
#include "utility/ProcessPath.h"
#include "resource.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct RegistWinCommand::PImpl
{
	HWND GetTargetWindow(HWND startHwnd)
	{
		HWND currentHwnd = startHwnd;
		DWORD currentPid = GetCurrentProcessId();
		for(;;) {
			HWND hwnd = GetWindow(currentHwnd, GW_HWNDNEXT);
			if (hwnd == NULL) {
				return NULL;
			}

			DWORD pid;
			GetWindowThreadProcessId(hwnd, &pid);
			if (pid == currentPid || IsWindowVisible(hwnd) == FALSE) {
				currentHwnd = hwnd;
				continue;
			}

			return hwnd;
		}
	}
	CommandRepository* mCmdMapPtr;
	CString mErrorMsg;
	uint32_t mRefCount;
};

RegistWinCommand::RegistWinCommand(CommandRepository* cmdMapPtr) : in(new PImpl)
{
	in->mRefCount = 1;
	in->mCmdMapPtr = cmdMapPtr;
}

RegistWinCommand::~RegistWinCommand()
{
	delete in;
}

CString RegistWinCommand::GetName()
{
	return _T("registwin");
}

CString RegistWinCommand::GetDescription()
{
	return _T("【アクティブウインドウ登録】");
}

BOOL RegistWinCommand::Execute()
{
	in->mErrorMsg.Empty();

	HWND hNextWindow =
	 	in->GetTargetWindow(AfxGetMainWnd()->GetSafeHwnd());
	if (hNextWindow == NULL) {
		in->mErrorMsg.LoadString(IDS_ERR_GETNEXTWINDOW);
		return FALSE;
	}

	ProcessPath processPath(hNextWindow);

	try {
		CString modulePath = processPath.GetProcessPath();
		CString moduleName = processPath.GetProcessName();
		// ウインドウタイトルを説明として使う
		CString description = processPath.GetCaption();
		CString param = processPath.GetCommandLine();
		in->mCmdMapPtr->NewCommandDialog(&moduleName, &modulePath, &description, &param);
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

BOOL RegistWinCommand::Execute(const std::vector<CString>& args)
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

soyokaze::core::Command* RegistWinCommand::Clone()
{
	return new RegistWinCommand(in->mCmdMapPtr);
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
