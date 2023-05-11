#include "pch.h"
#include "framework.h"
#include "commands/RegistWinCommand.h"
#include "CommandRepository.h"
#include "IconLoader.h"
#include "resource.h"
#include <psapi.h>
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
	std::map<CString, CString> mDeviceToDrive;
	CString mErrorMsg;
};

RegistWinCommand::RegistWinCommand(CommandRepository* cmdMapPtr) : in(new PImpl)
{
	in->mCmdMapPtr = cmdMapPtr;

	TCHAR deviceName[4096];
	// 変換表を作成しておく
	TCHAR path[] = { _T(' '), _T(':'), _T('\0') };
	for (TCHAR letter = _T('A'); letter <= _T('Z'); ++letter) {
		path[0] = letter;

		if (QueryDosDevice(path, deviceName, 4096) == 0) {
			continue;
		}
		in->mDeviceToDrive[deviceName] = path;
	}
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

	DWORD pid;
	GetWindowThreadProcessId(hNextWindow, &pid);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (hProcess == NULL) {
		in->mErrorMsg.LoadString(IDS_ERR_QUERYPROCESSINFO);

		CString msg;
		msg.Format(_T(" (PID:%d)"), pid);
		in->mErrorMsg += msg;

		return FALSE;
	}

	CString modulePath;
	TCHAR* p = modulePath.GetBuffer(32768);
	if (GetProcessImageFileName(hProcess, p, 32768) == 0) {
		modulePath.ReleaseBuffer();
		CloseHandle(hProcess);
		return TRUE;
	}
	modulePath.ReleaseBuffer();

	bool isReplaced = false;
	for (auto& item : in->mDeviceToDrive) {
		auto& deviceName = item.first;
		auto& driveLetter = item.second;

		if (_tcsncmp(deviceName, modulePath, deviceName.GetLength()) != 0) {
			continue;
		}

		isReplaced = true;
		modulePath = driveLetter + modulePath.Mid(deviceName.GetLength());
		break;
	}

	if (isReplaced == false) {
		modulePath.Empty();
	}

	// ウインドウタイトルを説明として使う
	CString caption;
	GetWindowText(hNextWindow, caption.GetBuffer(128), 128);
	caption.ReleaseBuffer();

	CloseHandle(hProcess);

	CString moduleName;
	if (modulePath.IsEmpty() == FALSE) {
		moduleName = PathFindFileName(modulePath);
		PathRemoveExtension(moduleName.GetBuffer(32768));
		moduleName.ReleaseBuffer();
	}

	in->mCmdMapPtr->NewCommandDialog(&moduleName, &modulePath, &caption);

	return TRUE;
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

BOOL RegistWinCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

Command* RegistWinCommand::Clone()
{
	return new RegistWinCommand(in->mCmdMapPtr);
}

