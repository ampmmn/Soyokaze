#pragma once

#include <memory>
#include <vector>
#include "matcher/Pattern.h"

namespace soyokaze {
namespace commands {
namespace bookmarks {

class ChromiumBrowseHistory
{
public:
	struct ITEM
	{
		CString mTitle;
		CString mUrl;
	};

public:
	ChromiumBrowseHistory(const CString& id, const CString& profileDir, bool isUseURL, bool isUseMigemo);
	~ChromiumBrowseHistory();

public:
	void Abort();
	void Query(Pattern* pattern, std::vector<ITEM>& items, int limit, DWORD timeout);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace soyokaze

