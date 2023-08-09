#pragma once

#include <memory>
#include <vector>

namespace soyokaze {
namespace commands {
namespace recentfiles {

struct ITEM {
	CString mName;
	CString mFullPath;
};

class RecentFiles
{
public:
	RecentFiles();
	~RecentFiles();

public:
	bool GetRecentFiles(std::vector<ITEM>& items);
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;


};


} // end of namespace recentfiles
} // end of namespace commands
} // end of namespace soyokaze

