#include "pch.h"
#include "NormalPriviledgeProcessProxy.h"
#include "SharedHwnd.h"
#include "SharedHwnd.h"
#include "utility/Path.h"
#include "utility/DemotedProcessToken.h"
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr LPCTSTR NAME_NORMALPRIV_SERVER = _T("LauncherAppNormalPriviledgeServer");

// 親の生存チェック用タイマー
constexpr int TIMERID_HEARTBEAT = 1;

namespace launcherapp {
namespace commands {
namespace common {

// サーバ(子プロセス)によって、通常権限で起動したコマンドプロセスのPIDを、
// 管理者特権で動作する親プロセス側に渡すための共有メモリクラス
class ProcessIDSharedMemory
{
	static constexpr int NumOfArrays = 4;
public:
	ProcessIDSharedMemory()
	{
		// 共有メモリ領域を作成する
		mMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DWORD) * NumOfArrays, _T("LauncherAppProcessIDShare"));
		if (mMapFile != nullptr) {
			mPID = (DWORD*) MapViewOfFile(mMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD) * NumOfArrays);
		}
	}
	~ProcessIDSharedMemory()
	{
		// 共有メモリ領域を破棄する
		if (mPID) {
			UnmapViewOfFile(mPID);
		}
		if (mMapFile) {
			CloseHandle(mMapFile);
		}
	}

	int IssueIndex() {
		int indexPID = mNextIndex++;
		if (mNextIndex >= NumOfArrays) {
			mNextIndex = 0;
		}
		return indexPID;
	}

	DWORD GetPID(int indexPID)
	{
		ASSERT(0 <= indexPID && indexPID < NumOfArrays);

		return mPID ? mPID[indexPID] : 0xFFFFFFFF;
	}

	static void RegisterPID(int indexPID, DWORD pid)
	{
		// 親プロセス側がひらいた共有メモリ領域を開く
		HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("LauncherAppProcessIDShare"));
		if (hMapFile == nullptr) {
			return;
		}
		auto pids = (DWORD*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD) * NumOfArrays);
		if (pids == nullptr) {
			return;
		}

		pids[indexPID] = pid;

		UnmapViewOfFile(pids);
		CloseHandle(hMapFile);
	}

private:
	HANDLE mMapFile = nullptr;
	DWORD* mPID = nullptr;
	int mNextIndex = 0;
};

struct COPYDATA_SHELLEXEC
{
	static constexpr int ID = 1;

	static void CreateFrom(int indexPID, const SHELLEXECUTEINFO* si, std::vector<uint8_t>& stm)
	{
		// 文字列系データの合計長を求める
		size_t dataLen = _tcslen(si->lpFile) + 1;

		if (si->lpParameters) {
			dataLen += (_tcslen(si->lpParameters) + 1);
		}
		if (si->lpDirectory) {
			dataLen += (_tcslen(si->lpDirectory) + 1);
		}

		// 求めた合計長を踏まえてデータ領域を確保する
		stm.resize(sizeof(COPYDATA_SHELLEXEC) + sizeof(TCHAR) * (dataLen));

		auto p = (COPYDATA_SHELLEXEC*)stm.data();


		p->mVersion = 1;
		p->mShowType = si->nShow;
		p->mIndexPID = indexPID;
		p->mParamOffset = -1;
		p->mWorkDirOffset = -1;


		// 文字列系データ(パス,パラメータ,作業ディレクトリ)をコピーするとともに、
		// オフセット値を計算してセットする
		int offset = 0;

		// パスをコピー
		size_t len = _tcslen(si->lpFile);
		memcpy(p->mData + offset, si->lpFile, sizeof(TCHAR) * (len + 1));
		p->mPathOffset = offset;
		offset += (int)(len + 1);

		// パラメータをコピー
		if (si->lpParameters) {
			len = _tcslen(si->lpParameters);
			memcpy(p->mData + offset, si->lpParameters, sizeof(TCHAR) * (len + 1));
			p->mParamOffset = offset;
			offset += (int)(len + 1);
		}

		// 作業ディレクトリのパスをコピー
		if (si->lpDirectory) {
			len = _tcslen(si->lpDirectory);
			memcpy(p->mData + offset, si->lpDirectory, sizeof(TCHAR) * (len + 1));
			p->mWorkDirOffset = offset;
			offset += (int)(len + 1);
		}
	}

	uint32_t mVersion = 0;
	int32_t mShowType = 0;
	int32_t mPathOffset = 0;
	int32_t mParamOffset = -1;
	int32_t mWorkDirOffset = -1;
	int32_t mIndexPID = 0;
	TCHAR mData[1] = {};
};

struct NormalPriviledgeProcessProxy::PImpl
{
	bool StartAgentProcessIfNeeded(HWND& hwndServer);
	bool StartAgentProcess();
	bool GetServerHwnd(HWND& hwnd);

	void RunAsNormalUser(COPYDATA_SHELLEXEC* param);

	ProcessIDSharedMemory* GetSharedPID()
	{
		if (mSharedPID.get() == nullptr) {
			mSharedPID.reset(new ProcessIDSharedMemory);
		}
		return mSharedPID.get(); 
	}

	std::mutex mMutex;
	std::unique_ptr<ProcessIDSharedMemory> mSharedPID;
};

// 通常権限で起動するためのプロセスが実行されていなかったら起動する
bool NormalPriviledgeProcessProxy::PImpl::StartAgentProcessIfNeeded(HWND& hwnd)
{
	std::lock_guard<std::mutex> lock(mMutex);

	HWND hwndServer = nullptr;
	if (GetServerHwnd(hwndServer) == false) {
		// 自分自身を通常権限で起動
		if (StartAgentProcess() == false) {
			return false;
		}

		// ウインドウがつくられるまで最大1秒待つ
		auto start = GetTickCount64();
		while(GetTickCount64() - start < 1000) {
			Sleep(50);
			if (GetServerHwnd(hwndServer) == false) {
				continue;
			}
			break;
		}

		if (hwndServer == nullptr) {
			SPDLOG_ERROR("Failed to get server window handle (timeout).");
			return false;
		}
	}

	hwnd = hwndServer;
	return true;
}

// 通常権限で起動するための処理をするためのプロセスを実行する
bool NormalPriviledgeProcessProxy::PImpl::StartAgentProcess()
{
	// 自分自身を起動
	Path pathSelf(Path::MODULEFILEPATH);

	STARTUPINFO si = {};
	si.cb = sizeof(si);
	si.dwFlags = 0;
	si.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION pi = {};

	CString cmdline(_T("--run-as-normal-priviledge-agent"));

	DemotedProcessToken tok;
	HANDLE htok = tok.FetchPrimaryToken();
	spdlog::debug(_T("primary token  {}"), (void*)htok);

	bool isRun = CreateProcessWithTokenW(htok, 0, pathSelf, cmdline.GetBuffer(MAX_PATH_NTFS), 0, nullptr, nullptr, &si, &pi);
	cmdline.ReleaseBuffer();

	if (pi.hThread) {
		CloseHandle(pi.hThread);
	}
	if (pi.hProcess) {
		CloseHandle(pi.hProcess);
	}

	if (isRun) {
		SPDLOG_INFO(_T("Start normal priviledge agent process. PID:{}"), pi.dwProcessId);
	}
	else {
		SPDLOG_ERROR(_T("Failed to run normal priviledge agent process!"));
	}

	return isRun;
}

bool NormalPriviledgeProcessProxy::PImpl::GetServerHwnd(HWND& hwnd)
{
	SharedHwnd sharedHwnd(NAME_NORMALPRIV_SERVER);
	HWND hwndProxy = sharedHwnd.GetHwnd(); 
	if (IsWindow(hwndProxy) == FALSE) {
		return false;
	}
	hwnd = hwndProxy;
	return true;
}

/**
 	通常ユーザ権限でプロセスを起動する
 	@param[in] param 起動するプロセスの情報
*/
void NormalPriviledgeProcessProxy::PImpl::RunAsNormalUser(COPYDATA_SHELLEXEC* param)
{
	ASSERT(param);

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
	}
	ProcessIDSharedMemory::RegisterPID(param->mIndexPID, pid);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



NormalPriviledgeProcessProxy::NormalPriviledgeProcessProxy() : in(new PImpl)
{
}

NormalPriviledgeProcessProxy::~NormalPriviledgeProcessProxy()
{
}

NormalPriviledgeProcessProxy* NormalPriviledgeProcessProxy::GetInstance()
{
	static NormalPriviledgeProcessProxy inst;
	return &inst;
}

bool NormalPriviledgeProcessProxy::StartProcess(SHELLEXECUTEINFO* si)
{
	// 通常権限で起動するためのプロセスが実行されていなかったら起動する
	HWND hwndServer = nullptr;
	if (in->StartAgentProcessIfNeeded(hwndServer) == false) {
		return false;
	}

	int indexPID = in->GetSharedPID()->IssueIndex();

	// SHELLEXECUTEINFOの情報からCOPYDATA_SHELLEXECを生成する
	std::vector<uint8_t> stm;
	COPYDATA_SHELLEXEC::CreateFrom(indexPID, si, stm);

	COPYDATASTRUCT copyData;
	copyData.dwData = COPYDATA_SHELLEXEC::ID;
	copyData.cbData = (DWORD)stm.size();
	copyData.lpData = (void*)stm.data();

	if (SendMessage(hwndServer, WM_COPYDATA, 0, (LPARAM)&copyData) == FALSE) {
		return false;
	}

	DWORD pid = in->GetSharedPID()->GetPID(indexPID);
	if (pid == 0xFFFFFFFF) {
		return false;
	}

	si->hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	return true;
}

LRESULT CALLBACK NormalPriviledgeProcessProxy::OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_COPYDATA) {
		COPYDATASTRUCT* data = (COPYDATASTRUCT*)lp;
		auto thisPtr = (NormalPriviledgeProcessProxy*)(size_t)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		thisPtr->OnCopyData(data);
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

void NormalPriviledgeProcessProxy::OnCopyData(COPYDATASTRUCT* data)
{
	if (data->dwData == COPYDATA_SHELLEXEC::ID) {
		// 通常権限でプロセス実行
		auto param = (COPYDATA_SHELLEXEC*)data->lpData;
		return in->RunAsNormalUser(param);
	}	
}


// 以下、通常権限で実行されたプロセス側から待ち受けようのウインドウを作成する
bool NormalPriviledgeProcessProxy::RunAgentUntilParentDie(int argc, TCHAR* argv[])
{
	bool isServerMode = false;
	for (int i = 0; i < argc; ++i) {
		if (_tcscmp(argv[i], _T("--run-as-normal-priviledge-agent")) == 0) {
			isServerMode = true;
			break;
		}
	}
	if (isServerMode == false) {
		return false;
	}

	CRect rc(0, 0, 0, 0);
	HINSTANCE hInst = AfxGetInstanceHandle();

	// 内部のmessage処理用の不可視のウインドウを作っておく
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("LncrNormalProviledgeProcessProxy"), 0, 
	                           rc.left, rc.top, rc.Width(), rc.Height(),
	                           NULL, NULL, hInst, NULL);
	ASSERT(hwnd);

	// 作成したウインドウのハンドルをサーバウインドウとして登録
	SharedHwnd serverHwnd(hwnd, NAME_NORMALPRIV_SERVER); 

	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

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

	return true;
}

}
}
}



