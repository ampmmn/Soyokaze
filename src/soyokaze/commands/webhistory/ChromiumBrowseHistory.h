#pragma once

#include <memory>
#include <vector>
#include "matcher/PatternInternal.h"

namespace launcherapp {
namespace commands {
namespace webhistory {

class ChromiumBrowseHistory
{
public:
	struct ITEM
	{
		CString mTitle;
		CString mUrl;
	};

public:
	ChromiumBrowseHistory();
	~ChromiumBrowseHistory();

public:
	bool Initialize(const CString& id, const CString& profileDir, bool isUseURL, bool isUseMigemo);
	bool Query(const std::vector<PatternInternal::WORD>& words, std::vector<ITEM>& items, int limit);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

