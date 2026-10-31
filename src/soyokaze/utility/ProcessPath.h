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
	/** PIDで指定したプロセスの実行ファイルを含むコマンドラインを取得する
	@return コマンドライン
	@param[in] processId プロセスID
	*/
	static CString GetFullCommandLine(DWORD processId);

protected:

protected:
	HWND mHwnd;

	CString mModuleFilePath;

};

