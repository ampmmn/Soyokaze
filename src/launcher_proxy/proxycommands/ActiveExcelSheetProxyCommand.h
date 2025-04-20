#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class ActiveExcelSheetProxyCommand : public ProxyCommand
{
public:
	ActiveExcelSheetProxyCommand();
	virtual ~ActiveExcelSheetProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(ActiveExcelSheetProxyCommand)
};

} // end of namespace 

