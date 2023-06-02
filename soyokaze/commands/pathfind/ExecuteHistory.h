#pragma once

#include <list>
#include <vector>

namespace soyokaze {
namespace commands {
namespace pathfind {

struct HISTORY_ITEM
{
	CString mWord;
	CString mFullPath;
};

class ExecuteHistory
{
public:
	ExecuteHistory();
	~ExecuteHistory();

	void Add(const CString& word, const CString& fullPath);
	void GetItems(std::vector<HISTORY_ITEM>& items) const;

	void Save();
	void Load();

private:
	std::list<HISTORY_ITEM> mItems;
};


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze



