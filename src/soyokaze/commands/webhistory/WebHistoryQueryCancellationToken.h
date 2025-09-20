#pragma once

#include "commands/webhistory/ChromiumBrowseHistory.h"
#include <mutex>

namespace launcherapp { namespace commands { namespace webhistory {

// キャンセルされたかどうかのフラグ管理
class QueryCancellationToken : public CancellationToken
{
public:
	QueryCancellationToken() = default;
	~QueryCancellationToken() = default;

	// 状態を初期化する
	void ResetState();

	// キャンセルが発生したか
	bool IsCancellationRequested() override;

private:
	std::mutex mMutex;
	uint64_t mStartTime{0};
};

}}}

