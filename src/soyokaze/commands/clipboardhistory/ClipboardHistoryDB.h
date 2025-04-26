#pragma once

#include <memory>
#include "commands/core/CommandQueryItem.h"
#include "commands/clipboardhistory/ClipboardHistoryEventListener.h"
#include "matcher/Pattern.h"

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

class ClipboardHistoryDB : public ClipboardHistoryEventListener
{
	using CommandQueryItemList = launcherapp::CommandQueryItemList;
public:
	// 検索結果
	struct ITEM {
		// 追加時刻
		uint64_t mAppendTime = 0;
		// データ
		CString mData;
	};
	using ResultList = std::vector<ITEM>;
public:
	ClipboardHistoryDB();
	~ClipboardHistoryDB();

	bool Load(int numResults, int sizeLimit, int countLimit);
	bool Unload();
	void UseRegExpSearch(bool useRegExp);
	void Query(Pattern* pattern, ResultList& result);


	//! クリップボードが更新された
	void UpdateClipboard(LPCTSTR data) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

