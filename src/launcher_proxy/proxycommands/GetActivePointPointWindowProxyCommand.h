#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class GetActivePointPointWindowProxyCommand : public ProxyCommand
{
public:
	GetActivePointPointWindowProxyCommand();
	virtual ~GetActivePointPointWindowProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(GetActivePointPointWindowProxyCommand)
};

} // end of namespace 

