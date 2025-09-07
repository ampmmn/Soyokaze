#pragma once

#include "commands/core/CommandQueryItemList.h"
#include <memory>
#include <vector>

namespace launcherapp { namespace commands { namespace webhistory {

class WebHistoryCommand
{
public:
	WebHistoryCommand();
	~WebHistoryCommand();

	bool Load();
	bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands);

	static uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames);
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::webhistory

