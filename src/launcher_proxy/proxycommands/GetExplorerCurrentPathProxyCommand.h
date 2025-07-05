#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class GetExplorerCurrentPathProxyCommand : public ProxyCommand
{
public:
	GetExplorerCurrentPathProxyCommand();
	virtual ~GetExplorerCurrentPathProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(GetExplorerCurrentPathProxyCommand)
};

} // end of namespace 

