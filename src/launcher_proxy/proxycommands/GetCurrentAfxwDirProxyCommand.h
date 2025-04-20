#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class GetCurrentAfxwDirProxyCommand : public ProxyCommand
{
public:
	GetCurrentAfxwDirProxyCommand();
	virtual ~GetCurrentAfxwDirProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(GetCurrentAfxwDirProxyCommand)
};

} // end of namespace 

