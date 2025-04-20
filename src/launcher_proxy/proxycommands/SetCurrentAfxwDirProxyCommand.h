#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class SetCurrentAfxwDirProxyCommand : public ProxyCommand
{
public:
	SetCurrentAfxwDirProxyCommand();
	virtual ~SetCurrentAfxwDirProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(SetCurrentAfxwDirProxyCommand)
};

} // end of namespace 

