// NormalPriviledgeAgent.cpp : 本体が管理者権限で動作している状況において、通常権限でコマンドを実行するための代理実行をする

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <cassert>
#include <spdlog/spdlog.h>
#include "NormalPriviledgeAgent.h"
#include "SharedHwnd.h"
#include "commands/share/NormalPriviledgeCopyData.h"
#include "commands/share/ProcessIDSharedMemory.h"



constexpr LPCTSTR NAME_NORMALPRIV_SERVER = _T("LauncherAppNormalPriviledgeServer");

// 親の生存チェック用タイマー
constexpr int TIMERID_HEARTBEAT = 1;

/**
 	通常ユーザ権限でプロセスを起動する
 	@param[in] param 起動するプロセスの情報
*/
static void RunAsNormalUser(COPYDATA_SHELLEXEC* param)
{
	assert(param);

	// 親プロセス側からうけとった情報に基づき、SHELLEXECUTEINFOを再構成する。
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = param->mShowType;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = param->mData + param->mPathOffset;
	if (param->mParamOffset != -1) {
		si.lpParameters = param->mData + param->mParamOffset;
	}
	if (param->mWorkDirOffset != -1) {
		si.lpDirectory = param->mData + param->mWorkDirOffset;
	}

	// 実行
	BOOL isRun = ShellExecuteEx(&si);

	// 起動したプロセスIDを呼び出し元に返す
	DWORD pid = 0xFFFFFFFF;
	if (si.hProcess) {
		pid = GetProcessId(si.hProcess);
		CloseHandle(si.hProcess);
		si.hProcess = nullptr;
	}
	ProcessIDSharedMemory::RegisterPID(param->mIndexPID, pid);
}


static void OnCopyData(COPYDATASTRUCT* data)
{
	if (data->dwData == COPYDATA_SHELLEXEC::ID) {
		// 通常権限でプロセス実行
		auto param = (COPYDATA_SHELLEXEC*)data->lpData;
		return RunAsNormalUser(param);
	}	
}



static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_COPYDATA) {
		COPYDATASTRUCT* data = (COPYDATASTRUCT*)lp;
		//auto thisPtr = (NormalPriviledgeProcessProxy*)(size_t)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		OnCopyData(data);
		return TRUE;
	}
	else if (msg == WM_TIMER && wp == TIMERID_HEARTBEAT) {
		SharedHwnd parentHwnd;
		if (IsWindow(parentHwnd.GetHwnd()) == FALSE) {
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}


int RunNormalPriviledgeAgent(HINSTANCE hInst)
{
	// 内部のmessage処理用の不可視のウインドウを作っておく
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("LncrNormalProviledgeProcessProxy"), 0, 
	                           0, 0, 0, 0,
	                           NULL, NULL, hInst, NULL);
	if (hwnd == nullptr) {
		return 1;
	}

	// 作成したウインドウのハンドルをサーバウインドウとして登録
	SharedHwnd serverHwnd(hwnd, NAME_NORMALPRIV_SERVER);

	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);
	//SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	SetTimer(hwnd, TIMERID_HEARTBEAT, 250, 0);

	for (;;) {
		MSG msg;
		int n = GetMessage(&msg, NULL, 0, 0); 
		if (n == 0 || n == -1) {
			break;
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return 0;
}

