#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class ActiveCalcSheetProxyCommand : public ProxyCommand
{
public:
	ActiveCalcSheetProxyCommand();
	virtual ~ActiveCalcSheetProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(ActiveCalcSheetProxyCommand)
};

} // end of namespace 

