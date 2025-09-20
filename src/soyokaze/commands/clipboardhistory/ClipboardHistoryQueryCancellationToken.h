#pragma once

#include "commands/clipboardhistory/ClipboardHistoryDB.h"

namespace launcherapp { namespace commands { namespace clipboardhistory {

// キャンセルされたかどうかのフラグ管理
class QueryCancellationToken : public CancellationToken
{
public:
	QueryCancellationToken() = default;
	~QueryCancellationToken() = default;

	// キャンセルが発生したか
	bool IsCancellationRequested() override;
};

}}}

