#pragma once

#include <memory>
#include "commands/remote/RemoteClientCommandParam.h"
#include <nlohmann/json.hpp>

namespace launcherapp { namespace commands { namespace remote {

class RemoteClient
{
public:
	RemoteClient();
	~RemoteClient();

	bool IsConnected();
	bool Connect(const CommandParam& param);
	bool Disconnect();
	CString GetErrorMessage();

	bool SendRequest(nlohmann::json& req);
	bool ReceiveResponse(nlohmann::json& res);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}} // end of namespace launcherapp::commands::remote

