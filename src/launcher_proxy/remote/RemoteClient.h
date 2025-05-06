#pragma once

#include <memory>

namespace launcherproxy { namespace remote {

class RemoteClient
{
public:
	RemoteClient();
	~RemoteClient();

	int Run(HINSTANCE hInst);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}} // end of namespace launcherproxy::remote

