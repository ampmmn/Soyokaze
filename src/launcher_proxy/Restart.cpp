// Restart.cpp : ランチャー再起動関連の処理
//

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
// Windows ヘッダー ファイル
#include <windows.h>
#include <shellapi.h>
#include <psapi.h>
#include <spdlog/spdlog.h>
#include "SharedHwnd.h"

constexpr int MAX_PATH_NTFS = (32767 + 1);

/**
 	ウインドウハンドルからアプリの実行ファイルのパスを得る
 	@return  true: 成功  false:失敗
 	@param[in]  h    ウインドウハンドル
 	@param[out] path 実行ファイルのパス
*/
static bool GetProcessPathFromWindow(HWND h, std::wstring& path)
{
	DWORD pid;
	GetWindowThreadProcessId(h, &pid);

	HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (process_handle == nullptr) {
		return false;
	}

	TCHAR full_path[MAX_PATH_NTFS];
	BOOL isOK = GetModuleFileNameEx(process_handle, nullptr, full_path, MAX_PATH_NTFS);
	CloseHandle(process_handle);

	if (isOK == FALSE) {
		return false;
	}
	path = full_path;
	return true;
}

/**
 	ランチャー本体にコマンドを送信する
 	@return 0:成功  1:失敗
 	@param[in] exe_path 本体exeのファイルパス
 	@param[in] cmd      送信するコマンド
*/
static int RunCommand(
	const std::wstring& exe_path,
	const wchar_t* cmd
)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_SHOW;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = exe_path.c_str();

	std::wstring cmdline(L"-c ");
	cmdline += cmd;
	si.lpParameters = cmdline.data();

	if (ShellExecuteEx(&si) == FALSE) {
		return 1;
	}
	
	if (si.hProcess) {
		CloseHandle(si.hProcess);
	}

	return 0;
}

int RestartApp()
{
	SharedHwnd hwnd;
	HWND h = hwnd.GetHwnd();
	if (IsWindow(h) == FALSE) {
		return 1;
	}

	// ①本体から実行ファイルのパスを得る
	std::wstring exe_path;
	if (GetProcessPathFromWindow(h, exe_path) == false) {
		return 1;
	}
	
	// ②指示を出す前にプロセスハンドルを取得しておく
	DWORD pid;
	GetWindowThreadProcessId(h, &pid);
	HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

	// ③シャットダウンの指示を出す
	RunCommand(exe_path, L"exit");
	// Note: exitコマンドは変更を許可しないため必ず存在するはず..

	// ④シャットダウン完了を待つ
	bool is_exited = false;

	constexpr DWORD64 SHUTDOWN_WAIT_TIMEOUT = 10000;  // 10秒
	DWORD64 s = GetTickCount64();
	while (GetTickCount64() - s < SHUTDOWN_WAIT_TIMEOUT) {

		DWORD exit_code = 0;
		if (GetExitCodeProcess(process_handle, &exit_code) && exit_code != STILL_ACTIVE) {
			is_exited = true;
			break;
		}

		Sleep(100);
	}
	CloseHandle(process_handle);
	process_handle = nullptr;

	if (is_exited == false) {
		// 終了を確認できなかった
		return 1;
	}

	// ⑤ 本体を再起動する(①で得たパスのexeを起動する)
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_SHOW;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = exe_path.c_str();

	TCHAR curDir[MAX_PATH_NTFS] = {};
	GetCurrentDirectory(MAX_PATH_NTFS, curDir);
	si.lpDirectory = curDir;

	if (ShellExecuteEx(&si) == FALSE) {
		return 1;
	}

	if (si.hProcess) {
		CloseHandle(si.hProcess);
	}
	return 0;
}
