#include "pch.h"
#include "ProcessPath.h"
//#include <subauth.h>
#include <winternl.h>
#include <psapi.h>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


ProcessPath::Exception::Exception(DWORD pid) : mPID(pid)
{

}

DWORD ProcessPath::Exception::GetPID()
{
	return mPID;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ProcessPath::ProcessPath(HWND hWnd) : mHwnd(hWnd)
{
}

ProcessPath::~ProcessPath()
{
}

/**
 * ウインドウハンドルが属するプロセスのパスを取得する
 * @return プロセスを生成する元となった実行ファイルのファイルパス
 */
CString ProcessPath::GetProcessPath()
{
	if (mModuleFilePath.IsEmpty() == FALSE) {
		return mModuleFilePath;
	}

	// ウインドウハンドルが属するプロセスIDを取得する
	DWORD pid;
	GetWindowThreadProcessId(mHwnd, &pid);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

	// ハンドルをとれなかったらエラーを返す
	if (hProcess == NULL) {
		throw Exception(pid);
	}

	CString modulePath;
	TCHAR* p = modulePath.GetBuffer(32768);
	DWORD copiedLen = GetProcessImageFileName(hProcess, p, 32768);
	modulePath.ReleaseBuffer();
	CloseHandle(hProcess);
	if (copiedLen == 0) {
		// パスが得られなかったらエラーを返す
		throw Exception(pid);
	}

	std::map<CString, CString> deviceToDrive;
	TCHAR deviceName[4096];
	// 変換表を作成
	TCHAR path[] = { _T(' '), _T(':'), _T('\0') };
	for (TCHAR letter = _T('A'); letter <= _T('Z'); ++letter) {
		path[0] = letter;

		if (QueryDosDevice(path, deviceName, 4096) == 0) {
			continue;
		}
		deviceToDrive[deviceName] = path;
	}

	// デバイス形式のパスをドライブレターから始まる絶対パスに変換する
	bool isReplaced = false;
	for (auto& item : deviceToDrive) {
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

	mModuleFilePath = modulePath;

	return mModuleFilePath;
}

CString ProcessPath::GetProcessName()
{
	// モジュールファイル名の拡張子を除いたものをプロセス名として返す
	CString modulePath = GetProcessPath();
	if (modulePath.IsEmpty()) {
		return _T("");
	}

	CString moduleName = PathFindFileName(modulePath);
	PathRemoveExtension(moduleName.GetBuffer(32768));
	moduleName.ReleaseBuffer();
	return moduleName;
}


CString ProcessPath::GetCaption()
{
	CString caption;
	GetWindowText(mHwnd, caption.GetBuffer(128), 128);
	caption.ReleaseBuffer();
	return caption;
}


DWORD ProcessPath::GetProcessId()
{
	DWORD pid;
	GetWindowThreadProcessId(mHwnd, &pid);
	return pid;
}

CString ProcessPath::GetCommandLine()
{
	// https://espresso3389.hatenablog.com/entry/20080723/1216815501

	struct RTL_USER_PROCESS_PARAMETERS_I {
		BYTE Reserved1[16];
		PVOID Reserved2[10];
		UNICODE_STRING ImagePathName;
		UNICODE_STRING CommandLine;
	};

	struct PEB_INTERNAL {
		BYTE Reserved1[2];
		BYTE BeingDebugged;
		BYTE Reserved2[1];
		PVOID Reserved3[2];
		struct PEB_LDR_DATA* Ldr;
		RTL_USER_PROCESS_PARAMETERS_I* ProcessParameters;
		BYTE Reserved4[104];
		PVOID Reserved5[52];
		struct PS_POST_PROCESS_INIT_ROUTINE* PostProcessInitRoutine;
		BYTE Reserved6[128];
		PVOID Reserved7[1];
		ULONG SessionId;
	};

	typedef NTSTATUS (NTAPI* NtQueryInformationProcessPtr)(
			IN HANDLE ProcessHandle,
			IN PROCESSINFOCLASS ProcessInformationClass,
			OUT PVOID ProcessInformation,
			IN ULONG ProcessInformationLength,
			OUT PULONG ReturnLength OPTIONAL);

	typedef ULONG (NTAPI* RtlNtStatusToDosErrorPtr)(NTSTATUS Status);

	// ウインドウハンドルが属するプロセスIDを取得する
	DWORD pid;
	GetWindowThreadProcessId(mHwnd, &pid);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

	HINSTANCE hNtDll = GetModuleHandleW(L"ntdll.dll");
	struct scope_freelib {
		scope_freelib(HANDLE hProcess, HINSTANCE h) : mP(hProcess), mH(h) {}
		~scope_freelib() {
			CloseHandle(mP);
			FreeLibrary(mH);
		}

		HANDLE mP;
		HINSTANCE mH;
	} scope_lib(hProcess, hNtDll);

	NtQueryInformationProcessPtr NtQueryInformationProcess = 
		(NtQueryInformationProcessPtr)GetProcAddress(hNtDll, "NtQueryInformationProcess");

	if(!NtQueryInformationProcess) {
		throw Exception(pid);
	}

	PROCESS_BASIC_INFORMATION pbi = {};
	ULONG len;
	NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &len);
	if (NT_ERROR(status) || !pbi.PebBaseAddress) {
		throw Exception(pid);
	}

	PEB_INTERNAL peb;
	size_t bytesRead = 0;
	if(!ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), &bytesRead)) {
		throw Exception(pid);
	}

	RTL_USER_PROCESS_PARAMETERS_I upp;
	if(!ReadProcessMemory(hProcess, peb.ProcessParameters, &upp, sizeof(upp), &bytesRead)) {
		throw Exception(pid);
	}

	// Check the buffer size
	len = (ULONG)upp.CommandLine.Length / sizeof(wchar_t) + 1;
	CStringW cmdline;
	LPWSTR buff = cmdline.GetBuffer(len);
	buff[len-1] = L'\0';

	if(!ReadProcessMemory(hProcess, upp.CommandLine.Buffer, buff, upp.CommandLine.Length, &bytesRead)) {
		cmdline.ReleaseBuffer();
		throw Exception(pid);
	}
	cmdline.ReleaseBuffer();

	cmdline.Trim();

	if (cmdline.IsEmpty()) {
		return _T("");
	}

	if (cmdline[0] == _T('"')) {
		int n = cmdline.Find(_T('"'), 1);
		if (n == -1) {
			return CString(cmdline);
		}
		return CString(cmdline.Mid(n+1));
	}
	else {
		int n = cmdline.Find(_T(' '), 0);
		if (n == -1) {
			return CString(cmdline);
		}
		return CString(cmdline.Mid(n+1));
	}
}
