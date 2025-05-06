// RemoteClient.cpp : リモート機能におけるクライアント側の機能を実装する

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include "RemoteClient.h"
#include "remote/ClientPipe.h"

#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )

namespace launcherproxy { namespace remote {

using json = nlohmann::json;

struct RemoteClient::PImpl
{
};


RemoteClient::RemoteClient() : in(new PImpl)
{
}

RemoteClient::~RemoteClient()
{
}

int RemoteClient::Run(HINSTANCE hInst)
{
	// 本体アプリとの通信用の名前付きパイプを開く
	ClientPipe pipe;

	// サーバに接続できなかったときに出力するエラー
	json connectionError;
	connectionError["result"]=false;
	connectionError["reason"]="Failed to connect to server.";

	// 接続したことを示す出力
	std::cout << "Hello" << std::endl;

	std::string input;
	while (std::getline(std::cin, input)) {

		if (input == "exit") {
			break;
		}

		// 改行を1つのリクエストの区切りとするため、改行を付与する
		input += "\n";

		// 入力された内容をパイプに書き込む(サーバに送る)
		if (pipe.SendRequest(input) == false) {
			connectionError["error_code"]=GetLastError();
			std::cout << connectionError.dump() << std::endl;
			continue;
		}
		// サーバからの応答を読み取る
		std::string response;
		if (pipe.ReceiveResponse(response) == false) {
			connectionError["error_code"]=GetLastError();
			std::cout << connectionError.dump() << std::endl;
			continue;
		}

		std::cout << response;
	}
	return 0;
}

}} // end of namespace launcherproxy::remote

