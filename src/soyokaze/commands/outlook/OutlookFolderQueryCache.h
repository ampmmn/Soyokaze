#pragma once

#include "commands/outlook/OutlookFolderQueryResult.h"
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace launcherapp { namespace commands { namespace outlook {

/** Outlookフォルダ一覧の非同期読み込みとキャッシュを管理する */
class OutlookFolderQueryCache
{
public:
	using EnumFoldersFunction = std::function<bool(std::vector<QueryResult>&)>;

	struct Snapshot
	{
		bool mIsEnable{false};
		bool mHasCache{false};
		bool mIsLoading{false};
		CString mPrefix;
		std::shared_ptr<const std::vector<QueryResult>> mQueryCache;
	};

	OutlookFolderQueryCache() = default;
	OutlookFolderQueryCache(const OutlookFolderQueryCache&) = delete;
	OutlookFolderQueryCache& operator=(const OutlookFolderQueryCache&) = delete;

	/**
	  デストラクタ。実行中の列挙スレッドの終了を待つ
	*/
	~OutlookFolderQueryCache()
	{
		if (mLoadThread.joinable()) {
			mLoadThread.join();
		}
	}

	/**
	  設定を更新し、必要な場合はバックグラウンドでフォルダ一覧を取得する
	  @param[in] isEnable Outlookフォルダ機能の有効状態
	  @param[in] prefix 検索プレフィックス
	  @param[in] enumFolders フォルダ一覧を取得する関数
	*/
	void Load(bool isEnable, const CString& prefix, EnumFoldersFunction enumFolders)
	{
		std::thread finishedThread;
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mIsEnable = isEnable;
			mPrefix = prefix;
			if (isEnable == false) {
				mQueryCache.reset();
				mHasCache = false;
				return;
			}
			if (mIsLoading) {
				return;
			}
			if (mLoadThread.joinable()) {
				finishedThread.swap(mLoadThread);
			}
			mIsLoading = true;
		}

		if (finishedThread.joinable()) {
			finishedThread.join();
		}

		try {
			std::lock_guard<std::mutex> lock(mMutex);
			if (mIsEnable == false) {
				mIsLoading = false;
				return;
			}
			mLoadThread = std::thread([this, enumFolders = std::move(enumFolders)]() {
				std::vector<QueryResult> results;
				std::shared_ptr<const std::vector<QueryResult>> cachedResults;
				bool isSuccess = false;
				try {
					isSuccess = enumFolders(results);
					if (isSuccess) {
						cachedResults = std::make_shared<const std::vector<QueryResult>>(std::move(results));
					}
				}
				catch (...) {
					isSuccess = false;
				}

				std::lock_guard<std::mutex> lock(mMutex);
				mIsLoading = false;
				if (mIsEnable && isSuccess) {
					mQueryCache = std::move(cachedResults);
					mHasCache = true;
				}
			});
		}
		catch (...) {
			std::lock_guard<std::mutex> lock(mMutex);
			mIsLoading = false;
		}
	}

	/**
	  キャッシュと設定のスナップショットを取得する
	  @return キャッシュと設定のスナップショット
	*/
	Snapshot GetSnapshot() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		Snapshot snapshot;
		snapshot.mIsEnable = mIsEnable;
		snapshot.mHasCache = mHasCache;
		snapshot.mIsLoading = mIsLoading;
		snapshot.mPrefix = mPrefix;
		if (mHasCache) {
			snapshot.mQueryCache = mQueryCache;
		}
		return snapshot;
	}

	/**
	  Outlookが終了している場合などにキャッシュを破棄する
	*/
	void Clear()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mQueryCache.reset();
		mHasCache = false;
	}

private:
	mutable std::mutex mMutex;
	std::shared_ptr<const std::vector<QueryResult>> mQueryCache;
	bool mHasCache{false};
	bool mIsEnable{false};
	bool mIsLoading{false};
	CString mPrefix;
	std::thread mLoadThread;
};

}}}
