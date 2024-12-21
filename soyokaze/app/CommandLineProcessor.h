#pragma once

#include "app/SecondProcessProxyIF.h"

namespace launcherapp {

class CommandLineProcessor
{
public:
	CommandLineProcessor() = default;
	~CommandLineProcessor() = default;

	bool Run(int argc, TCHAR* argv[], SecondProcessProxyIF* proxy);
};


}
