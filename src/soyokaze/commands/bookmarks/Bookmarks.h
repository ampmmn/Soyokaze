#pragma once

#include <memory>
#include "commands/bookmarks/BookmarkItem.h"

namespace launcherapp {
namespace commands {
namespace bookmarks {

class Bookmarks
{
public:
	Bookmarks();
	~Bookmarks();

	bool LoadChromeBookmarks(std::vector<Bookmark>& items);
	bool LoadEdgeBookmarks(std::vector<Bookmark>& items);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

