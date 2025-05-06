#include "ClientPipe.h"
#include <windows.h>

namespace launcherproxy { namespace remote {

constexpr LPCWSTR PIPE_PATH = L"\\\\.\\Global\\pipe\\LauncherAppRemoteProxy";

ClientPipe::ClientPipe()
{
}

ClientPipe::~ClientPipe()
{
	if (mPipeHandle) {
		CloseHandle(mPipeHandle);
		mPipeHandle = nullptr;
	}
}

// 入力された内容をパイプに書き込む(サーバに送る)
bool ClientPipe::SendRequest(const std::string& request)
{
	if (mPipeHandle == nullptr) {
		if (OpenPipe() == false) {
			return false;
		}
	}

	bool isErr = false; 
	size_t totalWritten = 0;
	while (totalWritten < request.size()) {
		DWORD written = 0;
		if (WriteFile(mPipeHandle, request.c_str() + totalWritten, (DWORD)(request.size() - totalWritten), &written, nullptr) == FALSE) {

			DWORD err = GetLastError();
			if (err != ERROR_BROKEN_PIPE && err != ERROR_NO_DATA && err != ERROR_PIPE_NOT_CONNECTED) {
				return false;
			}

			// パイプが閉じられたときは再度開いてやり直し
			CloseHandle(mPipeHandle);
			mPipeHandle = nullptr;
			if (OpenPipe() == false) {
				return false;
			}
			totalWritten = 0;
			continue;
		}
		totalWritten += written;
	}
	return true;
}

// サーバからの応答を読み取る
bool ClientPipe::ReceiveResponse(std::string& response)
{
	std::string res;

	while(res.find("\n") == std::string::npos) {

		// パイプからデータを読み取る
		char buff[512];
		DWORD read = 0;
		if (ReadFile(mPipeHandle, buff, 512, &read, nullptr) == FALSE) {
			return false;
		}
		// バッファに追記する
		res.insert(res.end(), buff, buff + read);
	}

	// 呼び出し元に結果を渡す
	response.swap(res);
	return true;
}

bool ClientPipe::OpenPipe()
{
	// 本体アプリとの通信用の名前付きパイプを開く
	HANDLE pipeHandle =	CreateFile(PIPE_PATH, GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (pipeHandle == INVALID_HANDLE_VALUE) {
		auto err = GetLastError();
		fprintf(stderr, "error: failed to open pipe err:%08x", err);
		return false;
	}

	mPipeHandle = pipeHandle;
	return true;
}

}} // end of namespace launcherproxy::remote
