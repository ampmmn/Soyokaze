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
#include <servprov.h>
#include <shobjidl_core.h>
#include <winrt/Windows.ApplicationModel.Core.h>

#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )

using json = nlohmann::json;

constexpr LPCTSTR PIPE_PATH = _T("\\\\.\\pipe\\LauncherAppNormalPriviledgePipe");
constexpr LPCTSTR NAME_NORMALPRIV_SERVER = _T("LauncherAppNormalPriviledgeServer");

// 親の生存チェック用タイマー
constexpr int TIMERID_HEARTBEAT = 1;

struct AdditionalEnvVariableSite : 
	winrt::implements<AdditionalEnvVariableSite, ::IServiceProvider, ::ICreatingProcess>
{
public:

	void SetEnvironmentVariables(const std::map<std::wstring, std::wstring>& vals) {
		mEnvMap = vals;
	}


	IFACEMETHOD(QueryService)(REFGUID service, REFIID riid, void** ppv) {
		if (service != SID_ExecuteCreatingProcess) {
			*ppv = nullptr;
			return E_NOTIMPL;
		}
		return this->QueryInterface(riid, ppv);
	}

	IFACEMETHOD(OnCreating)(ICreateProcessInputs* inputs) {

		for (auto& item : mEnvMap) {
			HRESULT hr = inputs->SetEnvironmentVariable(item.first.c_str(), item.second.c_str());
			if (hr != S_OK) {
				spdlog::error("SetEnvironmentVariable failed: {:x}", hr);
				break;
			}
		}

		return S_OK;
	}

private:
	std::map<std::wstring, std::wstring> mEnvMap;
};



struct NormalPriviledgeAgent::PImpl
{
	void ProcShellExecuteRequest(json& json_req);

	HANDLE mPipeHandle = nullptr;

};

static std::wstring& utf2utf(const std::string& src, std::wstring& dst)
{
	int cp = CP_UTF8;

	DWORD flags = MB_ERR_INVALID_CHARS;

	int requiredLen = MultiByteToWideChar(cp, flags, src.c_str(), -1, NULL, 0);
	if (requiredLen == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
		dst.clear();
		return dst;
	}

	dst.resize(requiredLen);

	MultiByteToWideChar(cp, flags, src.c_str(), -1, const_cast<wchar_t*>(dst.data()), requiredLen);
	return dst;
}

/**
 	通常ユーザ権限でプロセスを起動する
 	@param[in] json_req 起動するプロセスの情報
*/
void NormalPriviledgeAgent::PImpl::ProcShellExecuteRequest(json& json_req)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = json_req["show_type"];
	si.fMask = json_req["mask"];

	std::wstring file;
	si.lpFile = utf2utf(json_req["file"], file).c_str();

	std::wstring parameters;
	if (json_req.find("parameters") != json_req.end()) {
		std::string src = json_req["parameters"];
		si.lpParameters = utf2utf(src, parameters).c_str();
	}

	std::wstring directory;
	if (json_req.find("directory") != json_req.end()) {
		std::string src = json_req["directory"];
		si.lpParameters = utf2utf(src, directory).c_str();
	}

	std::map<std::wstring, std::wstring> env_map;
	if (json_req.find("environment") != json_req.end()) {
		std::wstring dst_key;
		std::wstring dst_value;
		auto dict = json_req.find("environment");
		for (auto it = dict->begin(); it != dict->end(); ++it) {
			env_map[utf2utf(it.key(), dst_key)] = utf2utf(it.value(), dst_value);
		}
	}

	// 追加の環境変数が設定されているか
	auto site = winrt::make_self<AdditionalEnvVariableSite>();
	site->SetEnvironmentVariables(env_map);
	if (env_map.empty() == false) {
		si.fMask |= SEE_MASK_FLAG_HINST_IS_SITE;
    si.hInstApp = reinterpret_cast<HINSTANCE>(site.get());
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


	// 結果を親プロセスに戻す
	json json_response;
	json_response["result"] = isRun != FALSE;
	json_response["pid"] = (int)pid;

	auto response_str = json_response.dump();
	response_str += "\n";
	size_t len = response_str.size() + 1;

	DWORD totalWrittenBytes = 0;
	while(totalWrittenBytes < len) {
		DWORD written = 0;
		if (WriteFile(mPipeHandle, response_str.c_str() + totalWrittenBytes, 
		              (DWORD)(len - totalWrittenBytes), 
		              &written, nullptr) == FALSE) {
			spdlog::error("Failed to write err:{}", GetLastError());
			return;
		}
		totalWrittenBytes += written;
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



NormalPriviledgeAgent::NormalPriviledgeAgent() : in(new PImpl)
{
}

NormalPriviledgeAgent::~NormalPriviledgeAgent()
{
}

// 通常権限で起動するための待ち受けサーバを起動する
int NormalPriviledgeAgent::Run(HINSTANCE hInst)
{
	// 内部のmessage処理用の不可視のウインドウを作っておく
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("LncrNormalProviledgeProcessProxy"), 0, 
	                           0, 0, 0, 0,
	                           NULL, NULL, hInst, NULL);
	if (hwnd == nullptr) {
		return 1;
	}

	// 親プロセスとの通信用の名前付きパイプを開く
	HANDLE pipeHandle = CreateFile(PIPE_PATH, GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (pipeHandle == INVALID_HANDLE_VALUE) {
		auto err = GetLastError();
		wchar_t msg[256];
		wsprintf(msg, L"fail err:%d", err);

		MessageBox(nullptr, msg, L"", 0);
		spdlog::error("Failed to create named pipe.");
		return 1;
	}

	in->mPipeHandle = pipeHandle;

	// 作成したウインドウのハンドルをサーバウインドウとして登録
	SharedHwnd serverHwnd(hwnd, NAME_NORMALPRIV_SERVER);
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
	SetTimer(hwnd, TIMERID_HEARTBEAT, 50, 0);

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

LRESULT CALLBACK
NormalPriviledgeAgent::OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_TIMER && wp == TIMERID_HEARTBEAT) {
		SharedHwnd parentHwnd;
		if (IsWindow(parentHwnd.GetHwnd()) == FALSE) {
			PostQuitMessage(0);
			return 0;
		}

		auto thisPtr = (NormalPriviledgeAgent*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		thisPtr->ProcRequest();
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

void NormalPriviledgeAgent::ProcRequest()
{
	auto pipe = in->mPipeHandle;

	DWORD len = 0;
	PeekNamedPipe(pipe, nullptr, 0, nullptr, nullptr, &len);
	if (len == 0) {
		return;
	}

	std::string request;
	char buff[512];

	for(;;) {
		DWORD read = 0;
		if (ReadFile(pipe, buff, 512, &read, nullptr) == FALSE) {
			return;
		}
		request.insert(request.end(), buff, buff + read);
		if (strchr(buff, '\n') == nullptr) {
			continue;
		}

		json json_req = json::parse(request);
		if (json_req.find("command") == json_req.end()) {
			// 不正なリクエスト
			return;
		}

		if (json_req["command"] == "shellexecute") {
			in->ProcShellExecuteRequest(json_req);
			break;
		}
	}

}

