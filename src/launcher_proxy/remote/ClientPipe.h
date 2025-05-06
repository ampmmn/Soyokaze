#pragma once

#include <Windows.h>
#include <string>

namespace launcherproxy { namespace remote {

class ClientPipe
{
public:
	ClientPipe();
	~ClientPipe();

	bool OpenPipe();

	bool SendRequest(const std::string& request);
	bool ReceiveResponse(std::string& response);

private:
	HANDLE mPipeHandle{nullptr};
};

}} // end of namespace launcherproxy::remote

