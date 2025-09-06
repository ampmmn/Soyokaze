#pragma once

#include <memory>
#include "commands/bookmarks/BookmarkItem.h"
#include "matcher/Pattern.h"

namespace launcherapp { namespace commands { namespace bookmarks {

class Bookmarks
{
public:
	Bookmarks();
	~Bookmarks();

	bool Initialize(LPCTSTR bookmarkPath);
	void SetNumOfKeywordShift(int num);
	void Query(Pattern* pattern, std::vector<Bookmark>& items, bool isUseURL);


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}} // end of namespace launcherapp::commands::bookmarks

