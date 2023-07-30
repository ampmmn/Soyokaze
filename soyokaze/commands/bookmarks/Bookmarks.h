#pragma once

#include <memory>

namespace soyokaze {
namespace commands {
namespace bookmarks {

struct ITEM
{
	CString mName;
	CString mUrl;
};

class Bookmarks
{
public:
	Bookmarks();
	~Bookmarks();

	bool LoadChromeBookmarks(std::vector<ITEM>& items);
	bool LoadEdgeBookmarks(std::vector<ITEM>& items);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace soyokaze

