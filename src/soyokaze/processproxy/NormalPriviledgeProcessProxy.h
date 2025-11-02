#pragma once

#include <cstdint>
#include <memory>
#include <map>
#include <string>
#include <vector>

#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )

namespace launcherapp { namespace processproxy {

class NormalPriviledgeProcessProxy
{
	NormalPriviledgeProcessProxy();
	~NormalPriviledgeProcessProxy();

public:
	static NormalPriviledgeProcessProxy* GetInstance();

	bool SendRequest(nlohmann::json& req, nlohmann::json& res);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}  // end of namespace launcherapp::processproxy

