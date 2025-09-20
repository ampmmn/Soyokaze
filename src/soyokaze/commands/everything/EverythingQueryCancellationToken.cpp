#include "pch.h"
#include "EverythingQueryCancellationToken.h"
#include "commands/core/CommandRepository.h"

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp { namespace commands { namespace everything {

constexpr int LIMIT_TIME = 250;   // 結果取得にかける時間(これを超過したら打ち切り)

// 状態を初期化する
void EverythingQueryCancellationToken::ResetState()
{
	// この時点での時刻を覚えておく
	std::scoped_lock<std::mutex> lock(mMutex);
	mLastQueryTime = GetTickCount64();
}

// キャンセルが発生したか?
bool EverythingQueryCancellationToken::IsCancellationRequested()
{
	auto repos = CommandRepository::GetInstance();
	if (repos->HasQueryRequest()) {
		// 後続のコマンド検索が来ていたら、前回のEverything検索を打ち切る
		return true;
	}
	// 一定時間経過したら打ち切り
	std::scoped_lock<std::mutex> lock(mMutex);
	return GetTickCount64() - mLastQueryTime > LIMIT_TIME;
}

}}}

