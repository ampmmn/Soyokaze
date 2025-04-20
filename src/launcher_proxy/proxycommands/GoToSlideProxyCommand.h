#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class GoToSlideProxyCommand : public ProxyCommand
{
public:
	GoToSlideProxyCommand();
	virtual ~GoToSlideProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(GoToSlideProxyCommand)
};

} // end of namespace 

