#pragma once

class ProcessPath
{
public:
	class Exception {
	public:
		Exception(DWORD pid);
		DWORD GetPID();

		DWORD mPID;
	};

public:
	ProcessPath(HWND hWnd);
	~ProcessPath();

public:
	CString GetProcessPath();
	CString GetProcessName();
	CString GetCaption();
	DWORD GetProcessId();
	CString GetCommandLine();

protected:

protected:
	HWND mHwnd;

	CString mModuleFilePath;

};

