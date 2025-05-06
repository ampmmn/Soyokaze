#include "pch.h"
#include "RemoteClient.h"
#include "utility/Path.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace remote {

using json = nlohmann::json;

struct RemoteClient::PImpl
{
	bool IsConnectionEstablished();
	bool IsSSHClosed();

	CString mErrMsg;
	HANDLE mProcess{nullptr};
	HANDLE mLocalWriter{nullptr};
	HANDLE mLocalReader{nullptr};
	bool mIsConnected{false};
};

bool RemoteClient::PImpl::IsConnectionEstablished()
{
	if (IsSSHClosed()) {
		return false;
	}

	// sshが返してきた内容で結果を判断する
	std::string str;

	for (;;) {
		DWORD read = 0;
		char buff[512];
		ReadFile(mLocalReader, buff, 512, &read, nullptr);
		buff[read] = '\0';

		const char* esc = "\x1b[?9001h\x1b[?1004h\0";
		if (read == 16 && memcmp(buff, esc, 16) == 0) {
			// 接続直後にでるエスケープシーケンスを読み飛ばす
			continue;
		}
		str.insert(str.end(), buff, buff + read);
		break;
	}

	if (str.find("Hello") != std::string::npos) {
		return true;
	}

	if (str.find("password") != std::string::npos) {
		mErrMsg = _T("認証が必要");
		return false;
	}
	else if (str.find("timed out") != std::string::npos) {
		mErrMsg = _T("接続がタイムアウトしました");
		return false;
	}
	else if (str.find("Host key verification failed.") != std::string::npos) {
		mErrMsg = _T("ホストキーの検証に失敗しました");
		return false;
	}

	std::wstring msg;
	UTF2UTF(str, msg);

	mErrMsg = _T("不明なエラー err:");
	mErrMsg += msg.c_str();
	return false;
}

bool RemoteClient::PImpl::IsSSHClosed()
{
	DWORD exitCode = 0;
	GetExitCodeProcess(mProcess, &exitCode);

	DWORD bytesAvailable = 0;
	char buff[1];
	BOOL result = PeekNamedPipe(mLocalReader, buff, 1, NULL, &bytesAvailable, NULL);

	return exitCode != STILL_ACTIVE && bytesAvailable == 0;
}


RemoteClient::RemoteClient() : in(new PImpl)
{
}

RemoteClient::~RemoteClient()
{
	Disconnect();
	if (in->mLocalWriter) {
		CloseHandle(in->mLocalWriter);
		in->mLocalWriter = nullptr;
	}
	if (in->mLocalReader) {
		CloseHandle(in->mLocalReader);
		in->mLocalReader = nullptr;
	}
	if (in->mProcess) {
		CloseHandle(in->mProcess);
	}
}

bool RemoteClient::IsConnected()
{
	return in->mIsConnected;
}

bool RemoteClient::Connect(const CommandParam& param)
{
	// OpenSSLはあるか?
	Path sslPath(Path::SYSTEMDIR, _T("OpenSSH\\ssh.exe"));
	if (sslPath.FileExists() == false) {
		in->mErrMsg = _T("ssh.exeがみつかりません");
		return false;
	}

	// ssh.exeとやり取りする用の名前なしパイプを作成する
	HANDLE hLocalReader = nullptr, hRemoteWriter = nullptr;
	CreatePipe(&hLocalReader, &hRemoteWriter, nullptr, 0);
	HANDLE hRemoteReader = nullptr, hLocalWriter = nullptr;
	CreatePipe(&hRemoteReader, &hLocalWriter, nullptr, 0);

	// 疑似コンソールを作成する
	COORD size = { 256, 4 };
	HPCON hPC = nullptr;
	CreatePseudoConsole(size, hRemoteReader, hRemoteWriter, 0, &hPC);

	// 子プロセスを作成するための準備
	STARTUPINFOEX siex = {};
	STARTUPINFO& si = siex.StartupInfo;
	si.cb = sizeof(siex);

	// lpAttributeListのサイズを得る
	SIZE_T attrSize = 0;
	InitializeProcThreadAttributeList(nullptr, 1, 0, &attrSize);

	// lpAttributeListの領域を割り当てる
	std::vector<uint8_t> buf(attrSize);
	siex.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)buf.data();

	// lpAttributeListを作成し、更新
	InitializeProcThreadAttributeList(siex.lpAttributeList, 1, 0, &attrSize);
	// 疑似コンソールのハンドルを設定する
	UpdateProcThreadAttribute(siex.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hPC, sizeof(HPCON),
	                          nullptr, nullptr);

	// ssh.exeを起動
	tstring commandline((LPCTSTR)sslPath);
	commandline += _T(" -o StrictHostKeyChecking=yes -o ConnectTimeout=1 ");
	commandline += _T(" \"");
	commandline += param.mHost;
	commandline += _T("\" \"");
	commandline += param.mLauncherAppDir;
	commandline += _T("\\launcher_proxy.exe\"");
	commandline += _T(" run-remote-client");

	PROCESS_INFORMATION pi = {};

	BOOL isOK = CreateProcessW(nullptr, (LPTSTR)commandline.c_str(), nullptr, nullptr, TRUE, 
	                           EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, &si, &pi); 

	// 不要なハンドルを閉じる（親側で使わない）
	CloseHandle(hRemoteReader);
	CloseHandle(hRemoteWriter);

	if (isOK == FALSE) {
		in->mErrMsg = _T("ssh.exeの実行に失敗しました");
		return false;
	}

	in->mLocalReader = hLocalReader;
	in->mLocalWriter = hLocalWriter;
	in->mProcess = pi.hProcess;
	in->mIsConnected = true;

	CloseHandle(pi.hThread);

	// 疎通確認をする
	return in->IsConnectionEstablished();
}

bool RemoteClient::Disconnect()
{
	if (in->mLocalWriter == nullptr) {
		return false;
	}

	DWORD written = 0;

	std::string buff("exit\n");
	WriteFile(in->mLocalWriter, buff.c_str(), (DWORD)buff.size(), &written, nullptr);

	in->mIsConnected = false;
	CloseHandle(in->mLocalWriter);
	in->mLocalWriter = nullptr;
	CloseHandle(in->mLocalReader);
	in->mLocalReader = nullptr;
	CloseHandle(in->mProcess);
	in->mProcess = nullptr;


	return written == buff.size();
}

CString RemoteClient::GetErrorMessage()
{
	return in->mErrMsg;
}

/**
 	リモートサーバーにリクエストを送る
 	@return true 成功  false 失敗
 	@param[out] res リクエストデータが格納されたJSONオブジェクト
*/
bool RemoteClient::SendRequest(json& req)
{
	if (in->mLocalWriter == nullptr) {
		spdlog::error("[remote] not connected.");
		return false;
	}

	// JSONデータを文字列化する
	auto reqStr = req.dump(-1, 32, true);
	// 区切り文字として改行を付与
	reqStr += "\r\n";

	// 送信
	DWORD written = 0;
	WriteFile(in->mLocalWriter, reqStr.c_str(), (DWORD)reqStr.size(), &written, nullptr);

	return written == reqStr.size();
}

/**
 	リモートサーバーからのレスポンスを受け取る
 	@return true 成功  false 失敗
 	@param[out] res レスポンスデータが格納されたJSONオブジェクト
*/
bool RemoteClient::ReceiveResponse(json& res)
{
	if (in->mLocalReader == nullptr) {
		spdlog::error("[remote] not connected.");
		return false;
	}

	std::string resStr;

	// エコーバックをスキップ
	while(resStr.find("\n") == std::string::npos) {
		if (in->IsSSHClosed()) {
			return false;
		}
		// パイプからデータを読み取る
		char buff[512];
		DWORD read = 0;
		if (ReadFile(in->mLocalReader, buff, 512, &read, nullptr) == FALSE) {
			return false;
		}
		// バッファに追記する
		resStr.insert(resStr.end(), buff, buff + read);
	}

	// 先頭行はエコーバックであるため、除去する
	auto pos = resStr.find("\n");
	if (pos == std::string::npos) {
		return false;
	}
	resStr = resStr.substr(pos + 1);

	// レスポンス部を読み取る
	std::string dbg;

	static std::regex patEnd("\\} *?\\r\?\n");
	while(std::regex_search(resStr, patEnd) == false) {
		if (in->IsSSHClosed()) {
			return false;
		}
		// パイプからデータを読み取る
		char buff[512];
		DWORD read = 0;
		if (ReadFile(in->mLocalReader, buff, 512, &read, nullptr) == FALSE) {
			return false;
		}
		// バッファに追記する
		resStr.insert(resStr.end(), buff, buff + read);

		// エスケープシーケンスを除去する
		static std::regex pat("(?:\\r\\n)?\x1b\\[\\d+;\\d+H.");
		dbg = resStr;
		resStr = std::regex_replace(resStr, pat, "");
		static std::regex pat2("\x1b\\[\\d+;\\d+H");
		resStr = std::regex_replace(resStr, pat2, "");
	}

	// 結果をJSONとしてパースする
	try {
		res = json::parse(resStr);
		return true;
	}
	catch(...) {
		spdlog::error("[remote]failed to parse response as JSON.");
		return false;
	}
}

}}} // end of namespace launcherapp::commands::remote

