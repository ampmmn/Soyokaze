#include "pch.h"
#include "WebHistoryQueryCancellationToken.h"
#include "commands/core/CommandRepository.h"

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp { namespace commands { namespace webhistory {

constexpr int LIMIT_TIME = 150;   // 結果取得にかける時間(これを超過したら打ち切り)

// 状態を初期化する
void QueryCancellationToken::ResetState()
{
	// この時点での時刻を覚えておく
	std::scoped_lock<std::mutex> lock(mMutex);
	mStartTime = GetTickCount64();
}

// キャンセルが発生したか?
bool QueryCancellationToken::IsCancellationRequested()
{
	auto repos = CommandRepository::GetInstance();
	if (repos->HasQueryRequest()) {
		// 後続のコマンド検索が来ていたら、前回のEverything検索を打ち切る
		return true;
	}
	// 一定時間経過したら打ち切り
	std::scoped_lock<std::mutex> lock(mMutex);
	return GetTickCount64() - mStartTime > LIMIT_TIME;
}

}}}

