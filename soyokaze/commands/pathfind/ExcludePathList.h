#pragma once

#include <set>

namespace soyokaze {
namespace commands {
namespace pathfind {

class ExcludePathList
{
public:
	ExcludePathList();
	~ExcludePathList();

	void Load();
	bool Contains(const CString& path);

private:
	std::set<CString> mFiles;
};


}
}
}
