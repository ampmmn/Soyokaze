#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/outlook/OutlookFolderQueryCache.h"
#include <atomic>
#include <chrono>
#include <future>
#include <thread>

using launcherapp::commands::outlook::OutlookFolderQueryCache;
using launcherapp::commands::outlook::QueryResult;

namespace {

// 非同期読み込みの完了を短時間待つ
bool WaitForLoadCompletion(OutlookFolderQueryCache& cache)
{
	for (int i = 0; i < 1000; ++i) {
		if (cache.GetSnapshot().mIsLoading == false) {
			return true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return false;
}

}

// 読み込み中はキャッシュを公開せず、完了後に結果を反映する
TEST(OutlookFolderProviderTest, PublishesCacheAfterBackgroundLoadCompletes)
{
	OutlookFolderQueryCache cache;
	std::promise<void> started;
	std::promise<void> resume;
	auto resumeFuture = resume.get_future().share();
	auto startedFuture = started.get_future();

	cache.Load(true, _T("outlook"), [&started, resumeFuture](std::vector<QueryResult>& results) {
		started.set_value();
		resumeFuture.wait();
		QueryResult result;
		result.mFullName = _T("Inbox");
		results.push_back(result);
		return true;
	});

	bool hasStarted = startedFuture.wait_for(std::chrono::seconds(1)) == std::future_status::ready;
	auto loadingSnapshot = cache.GetSnapshot();
	resume.set_value();

	EXPECT_TRUE(hasStarted);
	EXPECT_FALSE(loadingSnapshot.mHasCache);
	EXPECT_TRUE(WaitForLoadCompletion(cache));

	auto snapshot = cache.GetSnapshot();
	ASSERT_TRUE(snapshot.mHasCache);
	ASSERT_NE(nullptr, snapshot.mQueryCache);
	ASSERT_EQ(1u, snapshot.mQueryCache->size());
	EXPECT_EQ(_T("Inbox"), (*snapshot.mQueryCache)[0].mFullName);
	EXPECT_EQ(_T("outlook"), snapshot.mPrefix);
}

// 列挙失敗後は自動再試行せず、次回のLoadで再取得する
TEST(OutlookFolderProviderTest, RetriesOnlyWhenLoadIsCalledAgain)
{
	OutlookFolderQueryCache cache;
	std::atomic<int> callCount{0};
	auto enumFolders = [&callCount](std::vector<QueryResult>&) {
		++callCount;
		return false;
	};

	cache.Load(true, CString(), enumFolders);
	ASSERT_TRUE(WaitForLoadCompletion(cache));
	EXPECT_EQ(1, callCount.load());
	EXPECT_FALSE(cache.GetSnapshot().mHasCache);

	cache.Load(true, CString(), enumFolders);
	ASSERT_TRUE(WaitForLoadCompletion(cache));
	EXPECT_EQ(2, callCount.load());
	EXPECT_FALSE(cache.GetSnapshot().mHasCache);
}

// 無効化後に完了した列挙結果でキャッシュを復元しない
TEST(OutlookFolderProviderTest, DisablingDuringLoadPreventsCachePublication)
{
	OutlookFolderQueryCache cache;
	std::promise<void> started;
	std::promise<void> resume;
	auto resumeFuture = resume.get_future().share();
	auto startedFuture = started.get_future();

	cache.Load(true, CString(), [&started, resumeFuture](std::vector<QueryResult>& results) {
		started.set_value();
		resumeFuture.wait();
		results.push_back(QueryResult());
		return true;
	});

	bool hasStarted = startedFuture.wait_for(std::chrono::seconds(1)) == std::future_status::ready;
	cache.Load(false, CString(), [](std::vector<QueryResult>&) {
		return false;
	});
	resume.set_value();

	EXPECT_TRUE(hasStarted);
	EXPECT_TRUE(WaitForLoadCompletion(cache));
	auto snapshot = cache.GetSnapshot();
	EXPECT_FALSE(snapshot.mIsEnable);
	EXPECT_FALSE(snapshot.mHasCache);
	EXPECT_EQ(nullptr, snapshot.mQueryCache);
}
