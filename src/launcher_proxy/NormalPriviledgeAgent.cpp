// NormalPriviledgeAgent.cpp : 本体が管理者権限で動作している状況において、通常権限でコマンドを実行するための代理実行をする

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
#include <windows.h>
#include <shellapi.h>
#include <objbase.h>
#include <string>
#include <spdlog/spdlog.h>
#include "NormalPriviledgeAgent.h"
#include "NormalPriviledgeRequestBuffer.h"
#include "LncrNormalProviledgeProcessProxyWindow.h"
#include "logger/Logger.h"
#include "onenote/OneNoteAppProxy.h"
#include "ProxyCommandRepository.h"
#include "ProxyCommand.h"
#include "SharedHwnd.h"
#include "StringUtil.h"

using json = nlohmann::json;


/**
 * @brief NormalPriviledgeAgent の内部実装クラス
 */
struct NormalPriviledgeAgent::PImpl
{
	bool ReadPipeChunk(SharedHwnd& parentHwnd, char* buffer, DWORD bufferSize, DWORD& bytesRead);
	void SendResponse(json& json_res);

	HANDLE mPipeHandle = nullptr; ///< 名前付きパイプのハンドル
	HANDLE mPipeIoEvent{nullptr};
	NormalPriviledgeRequestBuffer mRequestBuffer;
	LncrNormalProviledgeProcessProxyWindow mProxyWindow;
};

/**
  非同期読み込みでデータを受信し、親ウィンドウの終了を監視する
  @param[in] parentHwnd 親ウィンドウの共有ハンドル
  @param[out] buffer 読み取ったデータの格納先
  @param[in] bufferSize 格納先のサイズ
  @param[out] bytesRead 読み取ったバイト数
  @return true:データを読み取った false:読み取り終了または失敗
*/
bool NormalPriviledgeAgent::PImpl::ReadPipeChunk(SharedHwnd& parentHwnd, char* buffer, DWORD bufferSize, DWORD& bytesRead)
{
	OVERLAPPED overlapped{};
	overlapped.hEvent = mPipeIoEvent;
	ResetEvent(mPipeIoEvent);
	bytesRead = 0;

	BOOL isRead = ReadFile(mPipeHandle, buffer, bufferSize, nullptr, &overlapped);
	if (isRead != FALSE) {
		return GetOverlappedResult(mPipeHandle, &overlapped, &bytesRead, FALSE) != FALSE && bytesRead > 0;
	}

	DWORD error = GetLastError();
	if (error == ERROR_MORE_DATA) {
		GetOverlappedResult(mPipeHandle, &overlapped, &bytesRead, FALSE);
		return bytesRead > 0;
	}
	if (error != ERROR_IO_PENDING) {
		spdlog::error("Failed to read pipe err:{}", error);
		return false;
	}

	for (;;) {
		DWORD waitResult = WaitForSingleObject(mPipeIoEvent, 50);
		if (waitResult == WAIT_OBJECT_0) {
			if (GetOverlappedResult(mPipeHandle, &overlapped, &bytesRead, FALSE) != FALSE) {
				return bytesRead > 0;
			}
			error = GetLastError();
			if (error == ERROR_MORE_DATA) {
				return bytesRead > 0;
			}
			if (error != ERROR_OPERATION_ABORTED) {
				spdlog::error("Failed to complete pipe read err:{}", error);
			}
			return false;
		}
		if (waitResult != WAIT_TIMEOUT) {
			CancelIoEx(mPipeHandle, &overlapped);
			WaitForSingleObject(mPipeIoEvent, INFINITE);
			GetOverlappedResult(mPipeHandle, &overlapped, &bytesRead, FALSE);
			spdlog::error("Failed to wait for pipe read.");
			return false;
		}
		if (IsWindow(parentHwnd.GetHwnd()) == FALSE) {
			CancelIoEx(mPipeHandle, &overlapped);
			WaitForSingleObject(mPipeIoEvent, INFINITE);
			GetOverlappedResult(mPipeHandle, &overlapped, &bytesRead, FALSE);
			return false;
		}
	}
}

void NormalPriviledgeAgent::PImpl::SendResponse(json& json_res)
{
	auto response_str = json_res.dump();
	response_str += "\n";
	size_t len = response_str.size() + 1;

	// 名前付きパイプに書き込む
	DWORD totalWrittenBytes = 0;
	while (totalWrittenBytes < len) {
		OVERLAPPED overlapped{};
		overlapped.hEvent = mPipeIoEvent;
		ResetEvent(mPipeIoEvent);
		DWORD written = 0;
		if (WriteFile(mPipeHandle, response_str.c_str() + totalWrittenBytes, 
		              (DWORD)(len - totalWrittenBytes), 
		              nullptr, &overlapped) == FALSE) {
			DWORD error = GetLastError();
			if (error == ERROR_IO_PENDING) {
				if (WaitForSingleObject(mPipeIoEvent, INFINITE) != WAIT_OBJECT_0 ||
				    GetOverlappedResult(mPipeHandle, &overlapped, &written, FALSE) == FALSE) {
					error = GetLastError();
					spdlog::error("Failed to write response err:{}", error);
					return;
				}
			}
			else if (error == ERROR_BROKEN_PIPE) {
				spdlog::info("Pipe has been closed.");
				return;
			}
			else {
				spdlog::error("Failed to write err:{}", error);
				return;
			}
		}
		else if (GetOverlappedResult(mPipeHandle, &overlapped, &written, FALSE) == FALSE) {
			spdlog::error("Failed to complete response write err:{}", GetLastError());
			return;
		}
		if (written == 0) {
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
 * @param pipeName 通信用の名前付きパイプ名
 * @return 実行結果
 */
int NormalPriviledgeAgent::Run(HINSTANCE hInst, LPCWSTR pipeName)
{
	if (pipeName == nullptr || *pipeName == L'\0') {
		MessageBox(nullptr, L"pipe name is not specified.", L"", 0);
		return 1;
	}

	// 親プロセスとの通信用の名前付きパイプを開く
	HANDLE pipeHandle = CreateFile(pipeName, GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
	if (pipeHandle == INVALID_HANDLE_VALUE) {
		auto err = GetLastError();
		wchar_t msg[256];
		wsprintf(msg, L"fail err:%d", err);

		MessageBox(nullptr, msg, L"", 0);
		spdlog::error("Failed to create named pipe.");
		return 1;
	}

	in->mPipeHandle = pipeHandle;
	in->mPipeIoEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (in->mPipeIoEvent == nullptr) {
		spdlog::error("Failed to create pipe I/O event.");
		CloseHandle(pipeHandle);
		in->mPipeHandle = nullptr;
		return 1;
	}

	if (in->mProxyWindow.Create(hInst) == false) {
		CloseHandle(in->mPipeIoEvent);
		in->mPipeIoEvent = nullptr;
		CloseHandle(pipeHandle);
		in->mPipeHandle = nullptr;
		return 1;
	}

	SharedHwnd parentHwnd;
	bool isBusy = false;
	for (;;) {
		if (IsWindow(parentHwnd.GetHwnd()) == FALSE) {
			break;
		}

		std::string request;
		if (in->mRequestBuffer.TryPopRequest(request)) {
			ProcRequest(request);
			continue;
		}
		bool hasPendingData = in->mRequestBuffer.HasPendingData();
		if (hasPendingData != isBusy) {
			isBusy = hasPendingData;
			in->mProxyWindow.SetBusy(isBusy);
		}

		char buffer[512];
		DWORD bytesRead = 0;
		if (in->ReadPipeChunk(parentHwnd, buffer, sizeof(buffer), bytesRead) == false) {
			if (IsWindow(parentHwnd.GetHwnd()) != FALSE) {
				spdlog::error("Stopping agent after named pipe read failure.");
			}
			break;
		}
		if (isBusy == false) {
			isBusy = true;
			in->mProxyWindow.SetBusy(true);
		}
		in->mRequestBuffer.Append(buffer, bytesRead);
	}

	CloseHandle(in->mPipeIoEvent);
	in->mPipeIoEvent = nullptr;
	CloseHandle(pipeHandle);
	in->mPipeHandle = nullptr;

	launcherproxy::OneNoteAppProxy::GetInstance()->Abort();

	return 0;
}

/**
 * @brief リクエストを処理する
 * @param request リクエスト文字列
 */
void NormalPriviledgeAgent::ProcRequest(const std::string& request)
{
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
		return;
	}

	auto str = json_res.dump();
	spdlog::debug(str);

	// 結果を返す
	in->SendResponse(json_res);
}

