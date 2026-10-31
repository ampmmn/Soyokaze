#include "pch.h"
#include "ProcessPath.h"
#include "utility/ScopeExit.h"
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
	TCHAR* p = modulePath.GetBuffer(MAX_PATH_NTFS);
	DWORD copiedLen = GetProcessImageFileName(hProcess, p, MAX_PATH_NTFS);
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
		auto& name = item.first;
		auto& driveLetter = item.second;

		if (_tcsncmp(name, modulePath, name.GetLength()) != 0) {
			continue;
		}

		isReplaced = true;
		modulePath = driveLetter + modulePath.Mid(name.GetLength());
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
	CString modulePath;
	// モジュールファイル名の拡張子を除いたものをプロセス名として返す
	try {
		modulePath = GetProcessPath();
	}
	catch (ProcessPath::Exception&) {
		return _T("");
	}

	CString moduleName = PathFindFileName(modulePath);
	PathRemoveExtension(moduleName.GetBuffer(MAX_PATH_NTFS));
	moduleName.ReleaseBuffer();
	return moduleName;
}


CString ProcessPath::GetCaption()
{
	CString caption;
	int len = GetWindowTextLengthW(mHwnd);
	if (len <= 0) {
		return _T("");
	}

	GetWindowText(mHwnd, caption.GetBuffer(len + 1), len + 1);
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
	CString cmdline = GetFullCommandLine(GetProcessId());
	cmdline.Trim();

	if (cmdline.IsEmpty()) {
		return _T("");
	}

	if (cmdline[0] == _T('"')) {
		int n = cmdline.Find(_T('"'), 1);
		if (n == -1) {
			return cmdline;
		}
		return cmdline.Mid(n + 1);
	}

	int n = cmdline.Find(_T(' '), 0);
	if (n == -1) {
		return cmdline;
	}
	return cmdline.Mid(n + 1);
}

/** PIDで指定したプロセスの実行ファイルを含むコマンドラインを取得する */
CString ProcessPath::GetFullCommandLine(DWORD processId)
{
	struct PEB32_INTERNAL {
		BYTE Reserved1[2];
		BYTE BeingDebugged;
		BYTE Reserved2[1];
		ULONG Reserved3[2];
		ULONG Ldr;
		ULONG ProcessParameters;
	};
	struct PEB64_INTERNAL {
		BYTE Reserved1[2];
		BYTE BeingDebugged;
		BYTE Reserved2[1];
		ULONG64 Reserved3[2];
		ULONG64 Ldr;
		ULONG64 ProcessParameters;
	};
	struct UNICODE_STRING32_INTERNAL {
		USHORT Length;
		USHORT MaximumLength;
		ULONG Buffer;
	};
	struct UNICODE_STRING64_INTERNAL {
		USHORT Length;
		USHORT MaximumLength;
		ULONG Padding;
		ULONG64 Buffer;
	};
	struct PROCESS_PARAMETERS32_INTERNAL {
		BYTE Reserved1[16];
		ULONG Reserved2[10];
		UNICODE_STRING32_INTERNAL ImagePathName;
		UNICODE_STRING32_INTERNAL CommandLine;
	};
	struct PROCESS_PARAMETERS64_INTERNAL {
		BYTE Reserved1[16];
		ULONG64 Reserved2[10];
		UNICODE_STRING64_INTERNAL ImagePathName;
		UNICODE_STRING64_INTERNAL CommandLine;
	};
	struct PROCESS_BASIC_INFORMATION64_INTERNAL {
		LONG ExitStatus;
		ULONG Padding;
		ULONG64 PebBaseAddress;
		ULONG64 AffinityMask;
		LONG BasePriority;
		ULONG Padding2;
		ULONG64 UniqueProcessId;
		ULONG64 InheritedFromUniqueProcessId;
	};
	typedef NTSTATUS (NTAPI* NtQueryInformationProcessPtr)(
		HANDLE ProcessHandle,
		PROCESSINFOCLASS ProcessInformationClass,
		PVOID ProcessInformation,
		ULONG ProcessInformationLength,
		PULONG ReturnLength);
	typedef NTSTATUS (NTAPI* NtWow64QueryInformationProcess64Ptr)(
		HANDLE ProcessHandle,
		PROCESSINFOCLASS ProcessInformationClass,
		PVOID ProcessInformation,
		ULONG ProcessInformationLength,
		PULONG ReturnLength);
	typedef NTSTATUS (NTAPI* NtWow64ReadVirtualMemory64Ptr)(
		HANDLE ProcessHandle,
		ULONG64 BaseAddress,
		PVOID Buffer,
		ULONG64 BufferSize,
		PULONG64 NumberOfBytesRead);
	typedef BOOL (WINAPI* IsWow64Process2Ptr)(HANDLE Process, USHORT* ProcessMachine, USHORT* NativeMachine);

	if (processId == 0) {
		throw Exception(processId);
	}

	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
	if (process == nullptr) {
		throw Exception(processId);
	}
	::utility::ScopeExit processGuard([process]() { CloseHandle(process); });

	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	if (ntdll == nullptr) {
		throw Exception(processId);
	}
	auto queryInformationProcess = reinterpret_cast<NtQueryInformationProcessPtr>(
		GetProcAddress(ntdll, "NtQueryInformationProcess"));
	if (queryInformationProcess == nullptr) {
		throw Exception(processId);
	}

	bool targetIs32Bit = false;
	auto isWow64Process2 = reinterpret_cast<IsWow64Process2Ptr>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "IsWow64Process2"));
	if (isWow64Process2 != nullptr) {
		USHORT processMachine = IMAGE_FILE_MACHINE_UNKNOWN;
		USHORT nativeMachine = IMAGE_FILE_MACHINE_UNKNOWN;
		if (isWow64Process2(process, &processMachine, &nativeMachine) == FALSE) {
			throw Exception(processId);
		}
		targetIs32Bit = processMachine == IMAGE_FILE_MACHINE_I386 || processMachine == IMAGE_FILE_MACHINE_ARMNT;
	}
	else {
		BOOL targetIsWow64 = FALSE;
		BOOL currentIsWow64 = FALSE;
		if (IsWow64Process(process, &targetIsWow64) == FALSE ||
			IsWow64Process(GetCurrentProcess(), &currentIsWow64) == FALSE) {
			throw Exception(processId);
		}
		targetIs32Bit = targetIsWow64 != FALSE || (sizeof(void*) == 4 && currentIsWow64 == FALSE);
	}

	ULONG64 pebAddress = 0;
	ULONG returnLength = 0;
	if (targetIs32Bit && sizeof(void*) == 8) {
		ULONG_PTR wow64PebAddress = 0;
		constexpr PROCESSINFOCLASS processWow64Information = static_cast<PROCESSINFOCLASS>(26);
		NTSTATUS status = queryInformationProcess(
			process,
			processWow64Information,
			&wow64PebAddress,
			sizeof(wow64PebAddress),
			&returnLength);
		if (status < 0 || wow64PebAddress == 0) {
			throw Exception(processId);
		}
		pebAddress = wow64PebAddress;
	}
	else if (targetIs32Bit == false && sizeof(void*) == 4) {
		auto queryInformationProcess64 = reinterpret_cast<NtWow64QueryInformationProcess64Ptr>(
			GetProcAddress(ntdll, "NtWow64QueryInformationProcess64"));
		if (queryInformationProcess64 == nullptr) {
			throw Exception(processId);
		}
		PROCESS_BASIC_INFORMATION64_INTERNAL processInfo{};
		NTSTATUS status = queryInformationProcess64(
			process,
			ProcessBasicInformation,
			&processInfo,
			sizeof(processInfo),
			&returnLength);
		if (status < 0 || processInfo.PebBaseAddress == 0) {
			throw Exception(processId);
		}
		pebAddress = processInfo.PebBaseAddress;
	}
	else {
		PROCESS_BASIC_INFORMATION processInfo{};
		NTSTATUS status = queryInformationProcess(
			process,
			ProcessBasicInformation,
			&processInfo,
			sizeof(processInfo),
			&returnLength);
		if (status < 0 || processInfo.PebBaseAddress == nullptr) {
			throw Exception(processId);
		}
		pebAddress = reinterpret_cast<ULONG_PTR>(processInfo.PebBaseAddress);
	}

	auto readMemory = [&](ULONG64 address, PVOID buffer, SIZE_T size) -> bool {
		if (targetIs32Bit == false && sizeof(void*) == 4) {
			auto readVirtualMemory64 = reinterpret_cast<NtWow64ReadVirtualMemory64Ptr>(
				GetProcAddress(ntdll, "NtWow64ReadVirtualMemory64"));
			if (readVirtualMemory64 == nullptr) {
				return false;
			}
			ULONG64 bytesRead = 0;
			NTSTATUS status = readVirtualMemory64(process, address, buffer, size, &bytesRead);
			return status >= 0 && bytesRead == size;
		}

		SIZE_T bytesRead = 0;
		BOOL result = ReadProcessMemory(
			process,
			reinterpret_cast<LPCVOID>(static_cast<ULONG_PTR>(address)),
			buffer,
			size,
			&bytesRead);
		return result != FALSE && bytesRead == size;
	};

	ULONG64 commandLineAddress = 0;
	USHORT commandLineLength = 0;
	if (targetIs32Bit) {
		PEB32_INTERNAL peb{};
		if (readMemory(pebAddress, &peb, sizeof(peb)) == false || peb.ProcessParameters == 0) {
			throw Exception(processId);
		}
		PROCESS_PARAMETERS32_INTERNAL parameters{};
		if (readMemory(peb.ProcessParameters, &parameters, sizeof(parameters)) == false) {
			throw Exception(processId);
		}
		commandLineAddress = parameters.CommandLine.Buffer;
		commandLineLength = parameters.CommandLine.Length;
	}
	else {
		PEB64_INTERNAL peb{};
		if (readMemory(pebAddress, &peb, sizeof(peb)) == false || peb.ProcessParameters == 0) {
			throw Exception(processId);
		}
		PROCESS_PARAMETERS64_INTERNAL parameters{};
		if (readMemory(peb.ProcessParameters, &parameters, sizeof(parameters)) == false) {
			throw Exception(processId);
		}
		commandLineAddress = parameters.CommandLine.Buffer;
		commandLineLength = parameters.CommandLine.Length;
	}

	if (commandLineLength == 0) {
		return _T("");
	}
	if (commandLineAddress == 0 || commandLineLength % sizeof(wchar_t) != 0) {
		throw Exception(processId);
	}

	int characterCount = commandLineLength / sizeof(wchar_t);
	CStringW commandLine;
	LPWSTR buffer = commandLine.GetBuffer(characterCount + 1);
	if (readMemory(commandLineAddress, buffer, commandLineLength) == false) {
		commandLine.ReleaseBuffer(0);
		throw Exception(processId);
	}
	buffer[characterCount] = L'\0';
	commandLine.ReleaseBuffer(characterCount);
	return CString(commandLine);
}
