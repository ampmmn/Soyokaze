#include "pch.h"
#include "ServerPipe.h"
#include "utility/DemotedProcessToken.h"
#include <memory>
#include <vector>
#include <sddl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace remote {

using json = nlohmann::json;

constexpr LPCTSTR PIPE_PATH = _T("\\\\.\\Global\\pipe\\LauncherAppRemoteProxy");

ServerPipe::ServerPipe()
{
}

ServerPipe::~ServerPipe()
{
	Close();
}

bool ServerPipe::IsOpen()
{
	return mPipeHandle != nullptr;
}

// クライアント側の要求を受けるための名前付きパイプを作成する
bool ServerPipe::Open()
{
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
		return false;
	}
	mPipeHandle = pipe;
	return true;
}

// 名前付きパイプを閉じる
void ServerPipe::Close()
{
	if (mPipeHandle) {
		DisconnectNamedPipe(mPipeHandle);
		CloseHandle(mPipeHandle);
		mPipeHandle = nullptr;
	}
}

bool ServerPipe::WaitForRequest(std::string& readBuffer)
{
	DWORD rest = 0;
	BOOL isOK = PeekNamedPipe(mPipeHandle, nullptr, 0, NULL, &rest, 0);
	if (rest == 0) {
		if (isOK == FALSE && GetLastError() == ERROR_BROKEN_PIPE) {
			// エラーが発生した場合は再度つなぎなおす
			spdlog::warn("[remote] broken pipe detected.");
			Close();
			Open();
		}
		return false;
	}

	DWORD read = 0;
	char buff[1024];
	if (ReadFile(mPipeHandle, buff, 1024, &read, nullptr) != FALSE) {
		// 読み取れたぶんを追記
		readBuffer.insert(readBuffer.end(), buff, buff + read);
		return true;
	}

	auto err = GetLastError();
	spdlog::warn("[remote] pipe err:{}", err);

	// エラーが発生した場合は再度つなぎなおす
	Close();
	Open();
	return true;
}

void ServerPipe::SendResponse(json& json_res)
{
	// JSONを文字列化する
	auto response_str = json_res.dump(-1, 32, true);

	// 区切りとしての改行を付与
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
				spdlog::info("[remote] pipe has been closed.");
				Close();
				Open();
				return;
			}
			spdlog::error("Failed to write err:{}", GetLastError());
			return;
		}
		totalWrittenBytes += written;
	}
}


}} // end of namespace launcherapp::remote

