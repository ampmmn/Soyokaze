#pragma once

#include "ProxyCommand.h"
#include "ProxyCommandRepository.h"
#include <memory>

namespace launcherproxy { 

class OneNoteGetHierarchyProxyCommand : public ProxyCommand
{
public:
	OneNoteGetHierarchyProxyCommand();
	virtual ~OneNoteGetHierarchyProxyCommand();

	std::string GetName() override;
	bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) override;

	DECLARE_PROXYCOMMAND(OneNoteGetHierarchyProxyCommand)

private:
		struct PImpl;
		std::unique_ptr<PImpl> in;
};

} // end of namespace 

