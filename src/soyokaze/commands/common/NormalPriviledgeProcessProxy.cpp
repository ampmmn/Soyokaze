// NormalPriviledgeAgent.cpp : 本体が管理者権限で動作している状況において、通常権限でコマンドを実行するための代理実行をする
#include "pch.h"
#include "NormalPriviledgeProcessProxy.h"
#include "SharedHwnd.h"
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

struct NormalPriviledgeProcessProxy::PImpl
{
	bool StartAgentProcessIfNeeded();
	bool StartAgentProcess();
	bool GetServerHwnd(HWND& hwnd);

	bool SendRequest(json& json);
	bool ReceiveResponse(json& json);

	std::mutex mMutex;
	HANDLE mPipeHandle{nullptr};
};

// 通常権限で起動するためのプロセスが実行されていなかったら起動する
bool NormalPriviledgeProcessProxy::PImpl::StartAgentProcessIfNeeded()
{
	std::lock_guard<std::mutex> lock(mMutex);

	// 通信用のパイプを作成する
	if (mPipeHandle == nullptr) {

		std::unique_ptr<SECURITY_ATTRIBUTES> sa;

		if (DemotedProcessToken::IsRunningAsAdmin()) {
			// 管理者権限で実行している場合は、
			// 通常権限で動作する子プロセスからパイプにアクセスできるようにするため、
			// セキュリティ記述子を設定する

			sa.reset(new SECURITY_ATTRIBUTES{sizeof(SECURITY_ATTRIBUTES)});
			sa->bInheritHandle = TRUE;

			// セキュリティ記述子を設定
			if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
						L"D:(A;;GA;;;WD)",  // ワールドアクセス許可
						SDDL_REVISION_1,
						&(sa->lpSecurityDescriptor),
						nullptr)) {

				spdlog::error("Failed to create securitydescriptor.");
				return false;
			}
		}

		auto pipe = CreateNamedPipe(PIPE_PATH, PIPE_ACCESS_DUPLEX, 
		                            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		                            PIPE_UNLIMITED_INSTANCES, 512, 512, 0, sa.get());
		if (pipe == INVALID_HANDLE_VALUE) {
			spdlog::error("Failed to create named pipe.");
			return false;
		}
		mPipeHandle = pipe;
	}

	// 接続を待つ
	HWND hwndServer{nullptr};
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

	bool isRun = false;
	if (DemotedProcessToken::IsRunningAsAdmin()) {
		// 管理者権限でアプリを場合は権限を降格させてプロセスを起動する
		DemotedProcessToken tok;
		HANDLE htok = tok.FetchPrimaryToken();
		spdlog::debug(_T("primary token  {}"), (void*)htok);

		isRun = CreateProcessWithTokenW(htok, 0, pathSelf, cmdline.GetBuffer(MAX_PATH_NTFS), 0, nullptr, nullptr, &si, &pi);
		cmdline.ReleaseBuffer();
	}
	else {
		// 通常権限でアプリを場合はそのままプロセスを起動する
		// (GetActiveObjectを使う機能を常にlauncher_proxy.exe経由で実行するため、
		//  通常権限でアプリを起動している場合もlaucnher_proxyを用いる)
		isRun = CreateProcess(pathSelf, cmdline.GetBuffer(MAX_PATH_NTFS), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
		cmdline.ReleaseBuffer();
	}

	if (isRun) {
		SPDLOG_INFO(_T("Start normal priviledge agent process. PID:{}"), pi.dwProcessId);
	}
	else {
		SPDLOG_ERROR(_T("Failed to run normal priviledge agent process!"));
	}

	if (pi.hThread) {
		CloseHandle(pi.hThread);
	}
	if (pi.hProcess) {
		CloseHandle(pi.hProcess);
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

bool NormalPriviledgeProcessProxy::PImpl::SendRequest(json& json_req)
{
	auto request_str = json_req.dump();
	request_str += "\n";
	size_t len = request_str.size() + 1;   // +1:nul終端分

	DWORD totalWrittenBytes = 0;
	while(totalWrittenBytes < len) {
		DWORD written = 0;
		if (WriteFile(mPipeHandle, request_str.c_str() + totalWrittenBytes, (DWORD)(len - totalWrittenBytes), 
		              &written, nullptr) == FALSE) {
			spdlog::error("Failed to write err:{}", GetLastError());
			return false;
		}
		totalWrittenBytes += written;
	}
	return true;
}

bool NormalPriviledgeProcessProxy::PImpl::ReceiveResponse(json& json_res)
{
	uint64_t start = GetTickCount64();

	std::string response_str;
	char buff[512];
	for(;;) {

		if (GetTickCount64()-start >= 2000) {
			spdlog::error("NormalPriviledgeProcessProxy::ReceiveResponse timeout.");
			return false;
		}

		DWORD read = 0;
		if (ReadFile(mPipeHandle, buff, 512, &read, nullptr) == FALSE) {
			spdlog::error("Failed to read err:{}", GetLastError());
			return false;
		}
		response_str.insert(response_str.end(), buff, buff + read);
		if (strchr(buff, '\n') != nullptr) {
			break;
		}
	}

	try {
		json_res = json::parse(response_str);
		return true;
	}
	catch(...) {
		spdlog::error("Failed to parse response json");
		return false;
	}
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
	 	const std::map<std::wstring, std::wstring>& envMap
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
	json_req["file"] = UTF2UTF(CString(si->lpFile), dst);
	if (si->lpParameters) {
		json_req["parameters"] = UTF2UTF(CString(si->lpParameters), dst);
	}
	if (si->lpDirectory) {
		json_req["directory"] = UTF2UTF(CString(si->lpDirectory), dst);
	}

	std::map<std::string, std::string> env_map;
	std::string dst_key;
	std::string dst_val;
	for (const auto& item : envMap) {
		env_map[UTF2UTF(item.first, dst_key)] = UTF2UTF(item.second, dst_val);
	}
	json_req["environment"] = env_map;

	// リクエストを送信する
	if (in->SendRequest(json_req) == false) {
		return false;
	}
	
	// 応答を待つ
	json json_res;
	if (in->ReceiveResponse(json_res) == false) {
		return false;
	}

	// 結果を取得する
	if (json_res.find("pid") == json_res.end()) {
		spdlog::error("unexpected response.");
		return false;
	}

	int pid = json_res["pid"];
	si->hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)pid);

	return true;
}

// あふwのカレントディレクトリを取得する
bool NormalPriviledgeProcessProxy::GetCurrentAfxwDir(std::wstring& path)
{
	// 通常権限で実行するためのプロセスが実行されていなかったら起動する
	if (in->StartAgentProcessIfNeeded() == false) {
		return false;
	}

	std::string dst;

	json json_req;
	json_req["command"] = "getcurrentafxwdir";

	// リクエストを送信する
	if (in->SendRequest(json_req) == false) {
		return false;
	}
	
	// 応答を待つ
	json json_res;
	if (in->ReceiveResponse(json_res) == false) {
		return false;
	}

	// 結果を取得
	if (json_res.find("path") == json_res.end()) {
		spdlog::error("unexpected response.");
		return false;
	}

	UTF2UTF((const std::string)json_res["path"], path);
	return true;
}

// あふwのカレントディレクトリを設定する
bool NormalPriviledgeProcessProxy::SetCurrentAfxwDir(const std::wstring& path)
{
	// 通常権限で実行するためのプロセスが実行されていなかったら起動する
	if (in->StartAgentProcessIfNeeded() == false) {
		return false;
	}

	std::string dst;

	json json_req;
	json_req["command"] = "setcurrentafxwdir";
	json_req["path"] = UTF2UTF(path, dst);

	// リクエストを送信する
	if (in->SendRequest(json_req) == false) {
		return false;
	}
	
	// 応答を待つ
	json json_res;
	if (in->ReceiveResponse(json_res) == false) {
		return false;
	}

	return true;
}

}
}
}



