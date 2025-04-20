#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class EnumCalcSheetsProxyCommand : public ProxyCommand
{
public:
	EnumCalcSheetsProxyCommand();
	virtual ~EnumCalcSheetsProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(EnumCalcSheetsProxyCommand)
};

} // end of namespace 

