// NormalPriviledgeAgent.cpp : 本体が管理者権限で動作している状況において、通常権限でコマンドを実行するための代理実行をする

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
#include <windows.h>
#include <shellapi.h>
#include <objbase.h>
#include <string>
#include <spdlog/spdlog.h>
#include "NormalPriviledgeAgent.h"
#include "logger/Logger.h"
#include "onenote/OneNoteAppProxy.h"
#include "ProxyCommandRepository.h"
#include "ProxyCommand.h"
#include "SharedHwnd.h"
#include "StringUtil.h"

using json = nlohmann::json;

constexpr LPCTSTR PIPE_PATH = _T("\\\\.\\pipe\\LauncherAppNormalPriviledgePipe");
constexpr LPCTSTR NAME_NORMALPRIV_SERVER = _T("LauncherAppNormalPriviledgeServer");

// 親の生存チェック用タイマー
constexpr int TIMERID_HEARTBEAT = 1;


/**
 * @brief NormalPriviledgeAgent の内部実装クラス
 */
struct NormalPriviledgeAgent::PImpl
{
	void SendResponse(json& json_res);

	HANDLE mPipeHandle = nullptr; ///< 名前付きパイプのハンドル
};

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
			DWORD err = GetLastError();
			if (err == 109) {
				spdlog::info("Pipe has been closed.");
				return;
			}
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
#ifdef _DEBUG
	Logger::Get()->Initialize();	
#endif

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

	CloseHandle(pipeHandle);
	in->mPipeHandle = nullptr;

	launcherproxy::OneNoteAppProxy::GetInstance()->Abort();

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
		// 改行が来るまでデータを読み取る
		DWORD read = 0;
		if (ReadFile(pipe, buff, 512, &read, nullptr) == FALSE) {

			DWORD err = GetLastError();
			if (err != 234) {
				return;
			}
			spdlog::debug("read more data read:{}", read);
		}
		request.insert(request.end(), buff, buff + read);
		if (strchr(buff, '\n') == nullptr) {
			continue;
		}

		// JSON リクエストを解析
		json json_req = json::parse(request);
		if (json_req.find("command") == json_req.end()) {
			// 不正なリクエスト
			spdlog::debug("command key does not exist.");
			return;
		}

		json json_res;

		// コマンド名に対応したコマンドオブジェクトを取得
		std::string command_name = json_req["command"];
		auto command_repository = launcherproxy::ProxyCommandRepository::GetInstance();

		spdlog::debug("command name:{}", command_name);

		// コマンドを実行
		auto proxy_command = command_repository->GetCommand(command_name);
		if (proxy_command == nullptr || proxy_command->Execute(json_req, json_res) == false) {
			spdlog::debug("Execute returned false.");
			json_res["result"] = false;
			in->SendResponse(json_res);
			break;
		}

		auto str = json_res.dump();
		spdlog::debug(str);

		// 結果を返す
		in->SendResponse(json_res);
		break;
	}
}

