#pragma once

#include "commands/explorepath/ExplorePathExtraActionSettings.h"
#include <memory>

namespace launcherapp { namespace commands { namespace shellexecute {

class ExtraActionSettings
{
	ExtraActionSettings();
	~ExtraActionSettings();

public:
	static ExtraActionSettings* Get();
	launcherapp::commands::explorepath::ExtraActionSettings* GetSettings();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}}


