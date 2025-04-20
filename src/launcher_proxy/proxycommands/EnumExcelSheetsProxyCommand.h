#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class EnumExcelSheetsProxyCommand : public ProxyCommand
{
public:
	EnumExcelSheetsProxyCommand();
	virtual ~EnumExcelSheetsProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(EnumExcelSheetsProxyCommand)
};

} // end of namespace 

