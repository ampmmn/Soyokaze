#pragma once

#include <vector>

namespace soyokaze {
namespace commands {
namespace common {

struct HISTORY_ITEM
{
	CString mWord;
	CString mFullPath;
};

class ExecuteHistory
{
public:
	using ItemList = std::vector<HISTORY_ITEM>;

private:
	ExecuteHistory();
	~ExecuteHistory();

public:
	static ExecuteHistory* GetInstance();

	void Add(const CString& type, const CString& word, const CString& fullPath);
	void Add(const CString& type, const CString& word);
	void GetItems(const CString& type, ItemList& items) const;

	void Save();
	void Load();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace common
} // end of namespace commands
} // end of namespace soyokaze



