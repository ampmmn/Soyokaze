#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"

namespace launcherproxy { 

class EnumPresentationSlidesProxyCommand : public ProxyCommand
{
public:
	EnumPresentationSlidesProxyCommand();
	virtual ~EnumPresentationSlidesProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(EnumPresentationSlidesProxyCommand)
};

} // end of namespace 

