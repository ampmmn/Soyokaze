#pragma once

#include "commands/core/UnknownIF.h"
#include "commands/core/CommandQueryItemList.h"
#include "matcher/Pattern.h"

namespace launcherapp {
namespace commands {
namespace core {

class ExtraCandidateSource : virtual public launcherapp::core::UnknownIF
{
public:
	virtual bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands) = 0;
	virtual void ClearCache() = 0;
};

}
}
}

