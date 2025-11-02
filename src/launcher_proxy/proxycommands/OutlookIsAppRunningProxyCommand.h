#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class OutlookIsAppRunningProxyCommand : public ProxyCommand
{
public:
	OutlookIsAppRunningProxyCommand();
	virtual ~OutlookIsAppRunningProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(OutlookIsAppRunningProxyCommand)
};

} // end of namespace 

