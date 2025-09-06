#pragma once

#include <vector>
#include <set>

namespace launcherapp {
namespace commands {
namespace common {

struct HISTORY_ITEM
{
	CString mWholeWord;
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

	void Add(const CString& type, const CString& word);
	void GetItems(const CString& type, ItemList& items) const;
	int EraseItems(const CString& type, const std::set<CString>& words);

	// 指定ワードを含む場合に履歴に追加しないようにする
	uint32_t AddExcludeWord(const CString& excludeWord);
	uint32_t RemoveExcludeWord(const CString& excludeWord);

	void ClearAllItems();

	void Save();
	void Load();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp



