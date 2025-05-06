#pragma once

#include <memory>

namespace launcherapp { namespace remote {

/**
リモートコマンドからの要求を受け取るためのサーバ
所定の名前付きパイプを開いてクライアントからの要求を待ち受ける
*/
class RemoteServer
{
public:
	RemoteServer();
	~RemoteServer();

	void Start();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}} // end of namespace launcherapp::remote

