#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"
#include <memory>

namespace launcherproxy { 

class OutlookEnumFoldersProxyCommand : public ProxyCommand
{
public:
	OutlookEnumFoldersProxyCommand();
	virtual ~OutlookEnumFoldersProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(OutlookEnumFoldersProxyCommand)

private:
		struct PImpl;
		std::unique_ptr<PImpl> in;
};

} // end of namespace 

