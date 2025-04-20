#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class ShellExecuteProxyCommand : public ProxyCommand
{
public:
	ShellExecuteProxyCommand();
	virtual ~ShellExecuteProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(ShellExecuteProxyCommand)
};

} // end of namespace 

