// NormalPriviledgeAgent.cpp : 本体が管理者権限で動作している状況において、通常権限でコマンドを実行するための代理実行をする

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <cassert>
#include <spdlog/spdlog.h>
#include "NormalPriviledgeAgent.h"
#include "commands/share/AfxWFunctions.h"
#include "SharedHwnd.h"
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

/**
 * @brief 環境変数を設定するためのクラス
 */
struct AdditionalEnvVariableSite : 
	winrt::implements<AdditionalEnvVariableSite, ::IServiceProvider, ::ICreatingProcess>
{
public:

	/**
	 * @brief 環境変数を設定する
	 * @param vals 環境変数のキーと値のマップ
	 */
	void SetEnvironmentVariables(const std::map<std::wstring, std::wstring>& vals) {
		mEnvMap = vals;
	}

	/**
	 * @brief サービスをクエリする
	 * @param service サービスの GUID
	 * @param riid インターフェース ID
	 * @param ppv インターフェースへのポインタ
	 * @return 実装されていない場合は E_NOTIMPL
	 */
	IFACEMETHOD(QueryService)(REFGUID service, REFIID riid, void** ppv) {
		if (service != SID_ExecuteCreatingProcess) {
			*ppv = nullptr;
			return E_NOTIMPL;
		}
		return this->QueryInterface(riid, ppv);
	}

	/**
	 * @brief プロセス作成時に呼び出される
	 * @param inputs プロセス作成の入力
	 * @return 成功時は S_OK
	 */
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
	std::map<std::wstring, std::wstring> mEnvMap; ///< 環境変数のマップ
};

/**
 * @brief UTF-8 文字列を UTF-16 文字列に変換する
 * @param src UTF-8 文字列
 * @param dst UTF-16 文字列
 * @return 変換後の UTF-16 文字列
 */
static std::wstring& utf2utf(const std::string& src, std::wstring& dst)
{
	int cp = CP_UTF8;
	DWORD flags = MB_ERR_INVALID_CHARS;

	// 必要なバッファサイズを計算
	int requiredLen = MultiByteToWideChar(cp, flags, src.c_str(), -1, NULL, 0);
	if (requiredLen == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
		dst.clear();
		return dst;
	}

	// バッファを確保して変換
	dst.resize(requiredLen);
	MultiByteToWideChar(cp, flags, src.c_str(), -1, const_cast<wchar_t*>(dst.data()), requiredLen);
	return dst;
}

static std::string& utf2utf(const std::wstring& src, std::string& dst)
{
	int cp = CP_UTF8;

	int requiredLen = WideCharToMultiByte(cp, 0, src.c_str(), -1, NULL, 0, 0, 0);

	dst.resize(requiredLen);
	WideCharToMultiByte(cp, 0, src.c_str(), -1, dst.data(), requiredLen, 0, 0);
	return dst;
}



/**
 * @brief NormalPriviledgeAgent の内部実装クラス
 */
struct NormalPriviledgeAgent::PImpl
{
	/**
	 * @brief ShellExecute リクエストを処理する
	 * @param json_req リクエストの JSON データ
	 */
	void ProcShellExecuteRequest(json& json_req);
	void ProcGetCurrentAfxwDirRequest(json& json_req);
	void ProcSetCurrentAfxwDirRequest(json& json_req);

	void SendResponse(json& json_res);

	HANDLE mPipeHandle = nullptr; ///< 名前付きパイプのハンドル
};

/**
 * @brief ShellExecute リクエストを処理する
 * @param json_req リクエストの JSON データ
 */
void NormalPriviledgeAgent::PImpl::ProcShellExecuteRequest(json& json_req)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = json_req["show_type"];
	si.fMask = json_req["mask"];

	// 実行ファイルのパスを設定
	std::wstring file;
	si.lpFile = utf2utf(json_req["file"], file).c_str();

	// パラメータを設定
	std::wstring parameters;
	if (json_req.find("parameters") != json_req.end()) {
		std::string src = json_req["parameters"];
		si.lpParameters = utf2utf(src, parameters).c_str();
	}

	// 作業ディレクトリを設定
	std::wstring directory;
	if (json_req.find("directory") != json_req.end()) {
		std::string src = json_req["directory"];
		si.lpDirectory = utf2utf(src, directory).c_str();
	}

	// 環境変数を設定
	std::map<std::wstring, std::wstring> env_map;
	if (json_req.find("environment") != json_req.end()) {
		std::wstring dst_key;
		std::wstring dst_value;
		auto dict = json_req.find("environment");
		for (auto it = dict->begin(); it != dict->end(); ++it) {
			env_map[utf2utf(it.key(), dst_key)] = utf2utf(it.value(), dst_value);
		}
	}

	// 追加の環境変数が設定されている場合
	auto site = winrt::make_self<AdditionalEnvVariableSite>();
	site->SetEnvironmentVariables(env_map);
	if (env_map.empty() == false) {
		si.fMask |= SEE_MASK_FLAG_HINST_IS_SITE;
		si.hInstApp = reinterpret_cast<HINSTANCE>(site.get());
	}

	// ShellExecuteEx を実行
	BOOL isRun = ShellExecuteEx(&si);

	// 起動したプロセス ID を取得
	DWORD pid = 0xFFFFFFFF;
	if (si.hProcess) {
		pid = GetProcessId(si.hProcess);
		CloseHandle(si.hProcess);
		si.hProcess = nullptr;
	}

	// 結果を JSON 形式で親プロセスに返す
	json json_response;
	json_response["result"] = isRun != FALSE;
	json_response["pid"] = (int)pid;

	SendResponse(json_response);
}

void NormalPriviledgeAgent::PImpl::ProcGetCurrentAfxwDirRequest(json& json_req)
{
	std::wstring curDir;
	bool result = AfxW_GetCurrentDir(curDir);

	std::string tmp;

	json json_response;
	json_response["path"] = utf2utf(curDir, tmp);
	json_response["result"] = result;

	SendResponse(json_response);
}

void NormalPriviledgeAgent::PImpl::ProcSetCurrentAfxwDirRequest(json& json_req)
{
	bool result = false;

	try {
		std::string path_str = json_req["path"]; 
		std::wstring path;
		utf2utf(path_str, path);

		result = AfxW_SetCurrentDir(path);
	}
	catch(...) { }

	json json_response;
	json_response["result"] = result;

	SendResponse(json_response);
}

void NormalPriviledgeAgent::PImpl::SendResponse(json& json_res)
{
	auto response_str = json_res.dump();
	response_str += "\n";
	size_t len = response_str.size() + 1;

	// 名前付きパイプに書き込む
	DWORD totalWrittenBytes = 0;
	while (totalWrittenBytes < len) {
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



/**
 * @brief コンストラクタ
 */
NormalPriviledgeAgent::NormalPriviledgeAgent() : in(new PImpl)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		SPDLOG_ERROR("Failed to CoInitialize!");
	}
}

/**
 * @brief デストラクタ
 */
NormalPriviledgeAgent::~NormalPriviledgeAgent()
{
	CoUninitialize();
}

/**
 * @brief 通常権限で起動するための待ち受けサーバを起動する
 * @param hInst インスタンスハンドル
 * @return 実行結果
 */
int NormalPriviledgeAgent::Run(HINSTANCE hInst)
{
	// 内部のメッセージ処理用の不可視ウィンドウを作成
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

	// 作成したウィンドウのハンドルをサーバウィンドウとして登録
	SharedHwnd serverHwnd(hwnd, NAME_NORMALPRIV_SERVER);
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
	SetTimer(hwnd, TIMERID_HEARTBEAT, 50, 0);

	// メッセージループ
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

/**
 * @brief ウィンドウプロシージャ
 * @param hwnd ウィンドウハンドル
 * @param msg メッセージ
 * @param wp WPARAM
 * @param lp LPARAM
 * @return 処理結果
 */
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

/**
 * @brief リクエストを処理する
 */
void NormalPriviledgeAgent::ProcRequest()
{
	auto pipe = in->mPipeHandle;

	// パイプにデータがあるか確認
	DWORD len = 0;
	PeekNamedPipe(pipe, nullptr, 0, nullptr, nullptr, &len);
	if (len == 0) {
		return;
	}

	// リクエストを読み取る
	std::string request;
	char buff[512];

	for (;;) {
		DWORD read = 0;
		if (ReadFile(pipe, buff, 512, &read, nullptr) == FALSE) {
			return;
		}
		request.insert(request.end(), buff, buff + read);
		if (strchr(buff, '\n') == nullptr) {
			continue;
		}

		// JSON リクエストを解析
		json json_req = json::parse(request);
		if (json_req.find("command") == json_req.end()) {
			// 不正なリクエスト
			return;
		}

		std::string command = json_req["command"];

		// ShellExecute コマンドを処理
		if (command == "shellexecute") {
			in->ProcShellExecuteRequest(json_req);
			break;
		}
		// getcurrentafxwdir コマンドを処理
		if (command == "getcurrentafxwdir") {
			in->ProcGetCurrentAfxwDirRequest(json_req);
			break;
		}
		// setcurrentafxwdir コマンドを処理
		if (command == "setcurrentafxwdir") {
			in->ProcSetCurrentAfxwDirRequest(json_req);
			break;
		}
	}
}
