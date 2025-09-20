#pragma once

#include "commands/everything/EverythingProxy.h"
#include <mutex>

namespace launcherapp { namespace commands { namespace everything {

// キャンセルされたかどうかのフラグ管理
class EverythingQueryCancellationToken : public CancellationToken
{
public:
	EverythingQueryCancellationToken() = default;
	~EverythingQueryCancellationToken() = default;

	// 状態を初期化する
	void ResetState();

	// キャンセルが発生したか
	bool IsCancellationRequested() override;

private:
	std::mutex mMutex;
	uint64_t mLastQueryTime{0};
};

}}}

