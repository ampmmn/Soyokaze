#include "pch.h"
#include "NormalPriviledgeProcessProxy.h"
#include "SharedHwnd.h"
#include "commands/share/NormalPriviledgeCopyData.h"
#include "commands/share/ProcessIDSharedMemory.h"
#include "utility/Path.h"
#include "utility/DemotedProcessToken.h"
#include <mutex>
#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )
#include <sddl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;

constexpr LPCTSTR PIPE_PATH = _T("\\\\.\\pipe\\LauncherAppNormalPriviledgePipe");
constexpr LPCTSTR NAME_NORMALPRIV_SERVER = _T("LauncherAppNormalPriviledgeServer");

// 親の生存チェック用タイマー
constexpr int TIMERID_HEARTBEAT = 1;

namespace launcherapp {
namespace commands {
namespace common {

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



struct NormalPriviledgeProcessProxy::PImpl
{
	bool StartAgentProcessIfNeeded();
	bool StartAgentProcess();
	bool GetServerHwnd(HWND& hwnd);

	std::mutex mMutex;
	HANDLE mPipeHandle = nullptr;
};

// 通常権限で起動するためのプロセスが実行されていなかったら起動する
bool NormalPriviledgeProcessProxy::PImpl::StartAgentProcessIfNeeded()
{
	std::lock_guard<std::mutex> lock(mMutex);

	// 通信用のパイプを作成する
	if (mPipeHandle == nullptr) {

    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES) };
    sa.bInheritHandle = TRUE;

		// セキュリティ記述子を設定
		if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
					L"D:(A;;GA;;;WD)",  // ワールドアクセス許可
					SDDL_REVISION_1,
					&sa.lpSecurityDescriptor,
					nullptr)) {

			spdlog::error("Failed to create securitydescriptor.");
			return false;
		}

		auto pipe = CreateNamedPipe(PIPE_PATH, PIPE_ACCESS_DUPLEX, 
		                            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		                            PIPE_UNLIMITED_INSTANCES, 512, 512, 0, &sa);
		if (pipe == INVALID_HANDLE_VALUE) {
			spdlog::error("Failed to create named pipe.");
			return false;
		}
		mPipeHandle = pipe;
	}

	// 接続を待つ
	HWND hwndServer = nullptr;
	if (GetServerHwnd(hwndServer) == false) {
		// 通常権限で起動するためのエージェントプロセスを起動する
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

	return true;
}

// 通常権限で起動するための処理をするためのプロセスを実行する
bool NormalPriviledgeProcessProxy::PImpl::StartAgentProcess()
{
	// 自分自身を起動
	Path pathSelf(Path::MODULEFILEDIR, _T("launcher_proxy.exe"));

	STARTUPINFO si = {};
	si.cb = sizeof(si);
	si.dwFlags = 0;
	si.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION pi = {};

	CString cmdline(_T("launcher_proxy.exe run-normal-priviledge-agent"));

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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



NormalPriviledgeProcessProxy::NormalPriviledgeProcessProxy() : in(new PImpl)
{
}

NormalPriviledgeProcessProxy::~NormalPriviledgeProcessProxy()
{
	if (in->mPipeHandle) {
		CloseHandle(in->mPipeHandle);
		in->mPipeHandle = nullptr;
	}
}

NormalPriviledgeProcessProxy* NormalPriviledgeProcessProxy::GetInstance()
{
	static NormalPriviledgeProcessProxy inst;
	return &inst;
}

bool NormalPriviledgeProcessProxy::StartProcess(
		SHELLEXECUTEINFO* si,
	 	const std::map<CString, CString>& envMap
)
{
	// 通常権限で起動するためのプロセスが実行されていなかったら起動する
	if (in->StartAgentProcessIfNeeded() == false) {
		return false;
	}

	std::string dst;

	json json_req;
	json_req["command"] = "shellexecute";
	json_req["show_type"] = (int)si->nShow;
	json_req["mask"] = si->fMask;
	json_req["file"] = si->lpFile;
	if (si->lpParameters) {
		json_req["parameters"] = UTF2UTF(si->lpParameters, dst);
	}
	if (si->lpDirectory) {
		json_req["directory"] = UTF2UTF(si->lpDirectory, dst);
	}

	std::map<std::string, std::string> env_map;
	std::string dst_key;
	std::string dst_val;
	for (const auto& item : envMap) {
		env_map[UTF2UTF(item.first, dst_key)] = UTF2UTF(item.second, dst_val);
	}
	json_req["environment"] = env_map;

	// リクエストを送信する
	auto request_str = json_req.dump();
	request_str += "\n";
	size_t len = request_str.size() + 1;   // +1:nul終端分

	DWORD totalWrittenBytes = 0;
	while(totalWrittenBytes < len) {
		DWORD written = 0;
		if (WriteFile(in->mPipeHandle, request_str.c_str() + totalWrittenBytes, (DWORD)(len - totalWrittenBytes), 
		              &written, nullptr) == FALSE) {
			spdlog::error("Failed to write err:{}", GetLastError());
			return false;
		}
		totalWrittenBytes += written;
	}
	
	// 応答を待つ
	std::string response_str;
	char buff[512];
	for(;;) {
		DWORD read = 0;
		if (ReadFile(in->mPipeHandle, buff, 512, &read, nullptr) == FALSE) {
			spdlog::error("Failed to read err:{}", GetLastError());
			return false;
		}
		response_str.insert(response_str.end(), buff, buff + read);
		if (strchr(buff, '\n') != nullptr) {
			break;
		}
	}

	auto json_res = json::parse(response_str);

	if (json_res.find("pid") == json_res.end()) {
		spdlog::error("unexpected response.");
		return false;
	}

	int pid = json_res["pid"];
	si->hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)pid);

	return true;
}

}
}
}



