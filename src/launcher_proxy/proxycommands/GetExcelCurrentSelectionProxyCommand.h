#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class GetExcelCurrentSelectionProxyCommand : public ProxyCommand
{
public:
	GetExcelCurrentSelectionProxyCommand();
	virtual ~GetExcelCurrentSelectionProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(GetExcelCurrentSelectionProxyCommand)
};

} // end of namespace 

