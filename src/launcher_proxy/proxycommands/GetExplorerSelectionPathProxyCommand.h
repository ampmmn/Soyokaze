#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class GetExplorerSelectionPathProxyCommand : public ProxyCommand
{
public:
	GetExplorerSelectionPathProxyCommand();
	virtual ~GetExplorerSelectionPathProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(GetExplorerSelectionPathProxyCommand)
};

} // end of namespace 

