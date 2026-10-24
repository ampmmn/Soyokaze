// NormalPriviledgeAgent.cpp : 本体が管理者権限で動作している状況において、通常権限でコマンドを実行するための代理実行をする
#include "pch.h"
#include "NormalPriviledgeProcessProxy.h"
#include "core/LauncherProcessContext.h"
#include "SharedHwnd.h"
#include "utility/Path.h"
#include "utility/DemotedProcessToken.h"
#include <mutex>
#include <sddl.h>
#include <vector>
#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;

constexpr LPCTSTR PIPE_NAME_PREFIX = _T("LauncherAppNormalPriviledgePipe-");
constexpr LPCTSTR NAME_NORMALPRIV_SERVER = _T("LauncherAppNormalPriviledgeServer");

namespace launcherapp { namespace processproxy {

namespace {

// パイプ名をランダムに生成する
// (固定名では他のプロセスから接続され、悪用される可能性があるため)
CString GeneratePipeName()
{
	GUID guid;
	CoCreateGuid(&guid);

	CString name;
	name.Format(_T("\\\\.\\pipe\\%s%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"),
		PIPE_NAME_PREFIX,
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return name;
}

// トークンからユーザーのSIDを取得する
bool GetUserSid(HANDLE token, CString& sid)
{
	DWORD size = 0;
	if (GetTokenInformation(token, TokenUser, nullptr, 0, &size) == FALSE) {
		// サイズ照会ではバッファ不足が正常な結果となる
		DWORD error = GetLastError();
		if (error != ERROR_INSUFFICIENT_BUFFER) {
			return false;
		}
	}
	if (size == 0) {
		return false;
	}

	std::vector<BYTE> buffer(size);
	if (GetTokenInformation(token, TokenUser, buffer.data(), size, &size) == FALSE) {
		return false;
	}

	auto tokenUser = reinterpret_cast<TOKEN_USER*>(buffer.data());
	LPWSTR sidString = nullptr;
	if (ConvertSidToStringSidW(tokenUser->User.Sid, &sidString) == FALSE) {
		return false;
	}

	sid = sidString;
	LocalFree(sidString);
	return true;
}

// パイプのセキュリティ記述子を作成する
// (実行ユーザーと、通常権限で動作するエージェントのユーザーのみをアクセス可能とする)
std::unique_ptr<SECURITY_ATTRIBUTES> CreatePipeSecurityAttributes()
{
	std::vector<CString> sids;

	// 実行中のユーザーのSID
	HANDLE token = nullptr;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token) != FALSE) {
		CString sid;
		if (GetUserSid(token, sid)) {
			sids.push_back(sid);
		}
		CloseHandle(token);
	}

	// 降格した権限で動作するエージェントのユーザーのSID
	DemotedProcessToken demotedToken;
	HANDLE primaryToken = demotedToken.FetchPrimaryToken();
	if (primaryToken != nullptr) {
		CString sid;
		if (GetUserSid(primaryToken, sid)) {
			sids.push_back(sid);
		}
	}

	if (sids.empty()) {
		return nullptr;
	}

	CString sddl(_T("D:"));
	for (auto& sid : sids) {
		sddl += _T("(A;;GA;;;");
		sddl += sid;
		sddl += _T(")");
	}
	// SYSTEMも許可する
	sddl += _T("(A;;GA;;;SY)");

	std::unique_ptr<SECURITY_ATTRIBUTES> sa;
	sa.reset(new SECURITY_ATTRIBUTES{sizeof(SECURITY_ATTRIBUTES)});
	sa->bInheritHandle = TRUE;

	if (ConvertStringSecurityDescriptorToSecurityDescriptorW(
			sddl, SDDL_REVISION_1, &(sa->lpSecurityDescriptor), nullptr) == FALSE) {
		spdlog::error("Failed to create securitydescriptor.");
		return nullptr;
	}

	return sa;
}

} // namespace

struct NormalPriviledgeProcessProxy::PImpl
{
	bool ReopenPipe();
	bool StartAgentProcessIfNeeded();
	bool StartAgentProcess();
	bool GetServerHwnd(HWND& hwnd);
	bool IsServerAvailable(HWND hwnd);

	bool SendRequest(json& json);
	bool ReceiveResponse(json& json);

	std::mutex mMutex;
	HANDLE mPipeHandle{nullptr};
	CString mPipeName;
	bool mIsAgentStarted{false};
};

// proxyと通信用のパイプを作成する
bool NormalPriviledgeProcessProxy::PImpl::ReopenPipe()
{
	if (mPipeHandle) {
		// 前のものがある場合は閉じる
		DisconnectNamedPipe(mPipeHandle);
		CloseHandle(mPipeHandle);
		mPipeHandle = nullptr;
	}

	// パイプ名はランダムに生成し、他のプロセスから接続できないようにする
	if (mPipeName.IsEmpty()) {
		mPipeName = GeneratePipeName();
	}

	std::unique_ptr<SECURITY_ATTRIBUTES> sa;

	if (DemotedProcessToken::IsRunningAsAdmin()) {

		// 管理者権限で実行している場合は、
		// 通常権限で動作する子プロセスからパイプにアクセスできるようにするため、
		// セキュリティ記述子を設定する
		// (実行ユーザーのみをアクセス可能とし、他のユーザーが
		//  管理者権限で動作しているランチャーを悪用できるようにしない)

		sa = CreatePipeSecurityAttributes();
		if (sa == nullptr) {
			spdlog::error("Failed to create securitydescriptor.");
			return false;
		}
	}

	auto pipe = CreateNamedPipe(mPipeName, PIPE_ACCESS_DUPLEX, 
			                        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			                        PIPE_UNLIMITED_INSTANCES, 512, 512, 0, sa.get());
	if (pipe == INVALID_HANDLE_VALUE) {
		spdlog::error("Failed to create named pipe.");
		return false;
	}

	mPipeHandle = pipe;
	SPDLOG_INFO("pipe for proxy created.");
	return true;
}

// 通常権限で起動するためのプロセスが実行されていなかったら起動する
bool NormalPriviledgeProcessProxy::PImpl::StartAgentProcessIfNeeded()
{
	// 接続を待つ
	HWND hwndServer{nullptr};

	if (mIsAgentStarted == false) {
		// 以前起動したエージェントのウィンドウを覚えておく
		// (以前のエージェントが動作中の場合にも、新しいエージェントの
		//  ウィンドウを待つようにするため)
		HWND hwndOldAgent{nullptr};
		GetServerHwnd(hwndOldAgent);

		// パイプ名はランダムに生成しているため、エージェントは毎回起動し直す
		// (パイプの接続状態を初期化する)
		if (ReopenPipe() == false) {
			return false;
		}

		// 通常権限で起動するためのエージェントプロセスを起動する
		if (StartAgentProcess() == false) {
			return false;
		}
		mIsAgentStarted = true;

		// ウインドウがつくられるまで最大1秒待つ
		auto start = GetTickCount64();
		while(GetTickCount64() - start < 1000) {
			Sleep(50);
			if (GetServerHwnd(hwndServer) == false) {
				continue;
			}
			// 以前のエージェントのウィンドウであれば待ち続ける
			if (hwndOldAgent != nullptr && hwndServer == hwndOldAgent) {
				continue;
			}
			break;
		}

		if (hwndServer == nullptr) {
			SPDLOG_ERROR("Failed to get server window handle (timeout).");
			return false;
		}
	}
	else {
		if (GetServerHwnd(hwndServer) == false) {
			SPDLOG_ERROR("Failed to get server window handle.");
			return false;
		}
	}

	if (IsServerAvailable(hwndServer) == false) {
		spdlog::info("Normal privilege agent is busy or not responding.");
		return false;
	}
	return true;
}

// 通常権限で起動するための処理をするためのプロセスを実行する
bool NormalPriviledgeProcessProxy::PImpl::StartAgentProcess()
{
	// プロキシを起動
	Path pathProxy(Path::MODULEFILEDIR, _T("launcher_proxy.exe"));

	STARTUPINFO si = {};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION pi = {};

	CString cmdline;
	cmdline.Format(_T("launcher_proxy.exe run-normal-priviledge-agent \"%s\""), (LPCTSTR)mPipeName);

	bool isRun = false;
	if (DemotedProcessToken::IsRunningAsAdmin()) {
		// 管理者権限でアプリを場合は権限を降格させてプロセスを起動する
		DemotedProcessToken tok;
		HANDLE htok = tok.FetchPrimaryToken();
		spdlog::debug(_T("primary token  {}"), (void*)htok);

		isRun = CreateProcessWithTokenW(htok, 0, pathProxy, cmdline.GetBuffer(MAX_PATH_NTFS), 0, nullptr, nullptr, &si, &pi);
		cmdline.ReleaseBuffer();
	}
	else {
		// 通常権限でアプリを場合はそのままプロセスを起動する
		// (GetActiveObjectを使う機能を常にlauncher_proxy.exe経由で実行するため、
		//  通常権限でアプリを起動している場合もlaucnher_proxyを用いる)
		isRun = CreateProcess(pathProxy, cmdline.GetBuffer(MAX_PATH_NTFS), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
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

/**
  サーバウィンドウにビジー状態を問い合わせる
  @param[in] hwnd サーバウィンドウのハンドル
  @return true:受付可能 false:ビジーまたは問い合わせ失敗
*/
bool NormalPriviledgeProcessProxy::PImpl::IsServerAvailable(HWND hwnd)
{
	DWORD_PTR result = 1;
	LRESULT sendResult = SendMessageTimeout(hwnd, WM_APP + 1, 0, 0, SMTO_ABORTIFHUNG, 100, &result);
	return sendResult != 0 && result == 0;
}

bool NormalPriviledgeProcessProxy::PImpl::SendRequest(json& json_req)
{
	// 通常権限で起動するためのプロセスが実行されていなかったら起動する
	if (StartAgentProcessIfNeeded() == false) {
		return false;
	}

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

		if (launcherapp::core::LauncherProcessContext::GetInstance()->IsShutdownInProgress()) {
			spdlog::error("shutdown is in progress.");
			return false;
		}
		if (GetTickCount64()-start >= 2000) {
			spdlog::error("NormalPriviledgeProcessProxy::ReceiveResponse timeout.");
			return false;
		}

		DWORD read = 0;
		if (ReadFile(mPipeHandle, buff, 512, &read, nullptr) == FALSE) {
			DWORD err = GetLastError();
			if (err != 234) {
				spdlog::error("Failed to read err:{}", err);
				return false;
			}
			else {
				spdlog::debug("read more data read:{}", read);
			}
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
	catch(const json::exception& e) {
		spdlog::error("Failed to parse response json: {}", e.what());
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

bool NormalPriviledgeProcessProxy::SendRequest(nlohmann::json& json_req, nlohmann::json& json_res)
{
	try {
		std::lock_guard<std::mutex> lock(in->mMutex);

		// リクエストを送信する
		if (in->SendRequest(json_req) == false) {
			return false;
		}

		// 応答を待つ
		if (in->ReceiveResponse(json_res) == false) {
			return false;
		}

		if (json_res["result"] == false) {
			return false;
		}
		return true;
	}
	catch(const json::exception& e) {
		spdlog::error("[NormalPriviledgeProcessProxy::SendRequest] Unexpected exception occurred. {}", e.what());
		return false;
	}

}


}}  // end of namespace launcherapp::processproxy

