#pragma once

#include <memory>
#include "commands/core/CommandQueryItemList.h"
#include "commands/clipboardhistory/ClipboardHistoryEventListener.h"
#include "matcher/Pattern.h"

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

// キャンセルされたかどうかのフラグ管理
class CancellationToken
{
public:
	virtual ~CancellationToken() = default;
	// キャンセルが発生したか
	virtual bool IsCancellationRequested() = 0;
};

class ClipboardHistoryDB : public ClipboardHistoryEventListener
{
	using CommandQueryItemList = launcherapp::CommandQueryItemList;
public:
	// 検索結果
	struct ITEM {
		// 一致レベル
		int mMatchLevel = -1;
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
	void SetCancellationToken(CancellationToken* token);
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

