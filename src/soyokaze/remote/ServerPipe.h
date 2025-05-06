#pragma once

#include <string>

#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )
#include <sddl.h>


namespace launcherapp { namespace remote {

/**
サーバ用のパイプクラス
*/
class ServerPipe
{
public:
	ServerPipe();
	~ServerPipe();

	bool IsOpen();
	bool Open();
	void Close();

	bool WaitForRequest(std::string& readBuffer);
	void SendResponse(nlohmann::json& json_res);

private:
	HANDLE mPipeHandle{nullptr};
};


}} // end of namespace launcherapp::remote

