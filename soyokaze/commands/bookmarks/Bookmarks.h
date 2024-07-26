#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace bookmarks {

enum BrowserType {
	Chrome,
	Edge,
};


struct Bookmark
{
	CString mName;
	CString mUrl;
	int mBrowser = 0;
	int mMatchLevel = 0;
};

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

