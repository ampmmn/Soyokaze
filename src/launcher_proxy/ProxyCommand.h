#pragma once

#include <string>

#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )

namespace launcherproxy {

class ProxyCommand
{
public:
	virtual ~ProxyCommand() {}

	virtual std::string GetName() = 0;
	virtual bool Execute(nlohmann::json& json_req, nlohmann::json& json_res) = 0;
};


}
