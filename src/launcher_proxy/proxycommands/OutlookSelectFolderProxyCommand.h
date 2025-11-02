#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"
#include <memory>

namespace launcherproxy { 

class OutlookSelectFolderProxyCommand : public ProxyCommand
{
public:
	OutlookSelectFolderProxyCommand();
	virtual ~OutlookSelectFolderProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(OutlookSelectFolderProxyCommand)

private:
		struct PImpl;
		std::unique_ptr<PImpl> in;
};

} // end of namespace 

