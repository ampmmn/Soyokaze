#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class GetAfxwSelectionPathProxyCommand : public ProxyCommand
{
public:
	GetAfxwSelectionPathProxyCommand();
	virtual ~GetAfxwSelectionPathProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(GetAfxwSelectionPathProxyCommand)
};

} // end of namespace 

