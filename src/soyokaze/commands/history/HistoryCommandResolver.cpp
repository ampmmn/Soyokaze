#include "pch.h"
#include "HistoryCommandResolver.h"
#include "commands/history/HistoryCommandQueryRequest.h"
#include "commands/core/CommandRepository.h"
#include <map>
#include "resource.h"

using CommandRepository = launcherapp::core::CommandRepository;
using CommandQueryResult = launcherapp::commands::core::CommandQueryResult;
using Command = launcherapp::core::Command;

namespace launcherapp { namespace commands { namespace history {

struct CACHE_ITEM
{
	RefPtr<Command> mCommand;
	uint64_t mLastUsedTime{0};
};

// キャッシュを保持する期間(20分)
constexpr uint64_t CACHE_EXPIRED_PERIOD = 20 * 60 * 1000;

struct HistoryCommandResolver::PImpl
{
	void ClearExpiredCache();
	bool HitTestCache(const CString& keyword, Command** command);

	// キャッシュ
	std::map<CString, CACHE_ITEM> mCacheMap;
	// 対応する実際のコマンドが存在しないことが分かっている履歴キーワード
	std::map<CString, DWORD64> mUnavailableKeywordMap;    // キー:キーワード 値:最後に参照した時刻
	// 検索タイムアウト(msec)
	uint32_t mTimeout{2000};
	// キャッシュ切れをチェックした時間
	uint64_t mLastCacheCheckTime{0};
};

void HistoryCommandResolver::PImpl::ClearExpiredCache()
{
	auto now = GetTickCount64(); 
	// 前回チェックからすくなくとも1分は開ける
	constexpr uint64_t CHECK_INTERVAL = 1 * 60 * 1000; // 1分
	if (now - mLastCacheCheckTime < CHECK_INTERVAL) {
		return;
	}

	auto it = mCacheMap.begin();
	while (it != mCacheMap.end()) {
		auto& cacheItem = it->second;
		if (now - cacheItem.mLastUsedTime < CACHE_EXPIRED_PERIOD) {
			// まだ期限切れではない
			it++;
			continue;
		}
		// 期限切れを削除
		spdlog::debug(_T("history : cache expired {}"), (LPCTSTR)it->first);
		it = mCacheMap.erase(it);
	}
}

bool HistoryCommandResolver::PImpl::HitTestCache(const CString& keyword, Command** command)
{
	// まずキャッシュとして情報が登録されているかどうかを見る
	auto it = mCacheMap.find(keyword);
	if (it != mCacheMap.end()) {
		// キャッシュあり
		auto& cacheItem = it->second;
		auto cmd = cacheItem.mCommand.get();
		cmd->AddRef();
		*command = cmd;
		spdlog::debug(_T("history : hittest : hit+ {}"), (LPCTSTR)keyword);
		return true;
	}

	// 次に、「データが存在しないこと」として登録されているかを確認する
	auto itUnavailable = mUnavailableKeywordMap.find(keyword);
	if (itUnavailable != mUnavailableKeywordMap.end()) {

		auto now = GetTickCount64();
		if (now - itUnavailable->second < CACHE_EXPIRED_PERIOD) {
			// 存在しないことが分かっているので検索をスキップ
			itUnavailable->second = now;
			*command = nullptr;
			spdlog::debug(_T("history : hittest : hit- {}"), (LPCTSTR)keyword);
			return true;
		}

		// 前回は存在しなかったが、ある程度時間がたっているので検索を再度試みる
		mUnavailableKeywordMap.erase(itUnavailable);
	}
	// キャッシュヒットせず(要検索)
	return false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

HistoryCommandResolver::HistoryCommandResolver() : in(new PImpl)
{
}

HistoryCommandResolver::~HistoryCommandResolver()
{
}

// インスタンスを取得する
HistoryCommandResolver* HistoryCommandResolver::GetInstance()
{
	static HistoryCommandResolver inst;
	return &inst;
}

// キーワードに該当するコマンドを得る
bool HistoryCommandResolver::Resolve(const CString& keyword, Command** command)
{
	// 期限切れのキャッシュがあったら削除する
	in->ClearExpiredCache();

	// キャッシュに登録されていればそれを流用する
	if (in->HitTestCache(keyword, command)) {
		return *command != nullptr;
	}

	// 履歴キーワードによる絞り込みを実施
	RefPtr<CommandQueryRequest> req(new CommandQueryRequest(keyword));
	CommandRepository::GetInstance()->Query(req.get());

	// 検索完了を待つ
	if (req->WaitComplete(in->mTimeout) == false) {
		// タイムアウトするケースでは次回は検索をスキップする
		spdlog::error(_T("HISTORY: query timeout occurred. keyword:{}"), (LPCTSTR)keyword);
		in->mUnavailableKeywordMap[keyword] = GetTickCount64();
		return false;
	}

	RefPtr<CommandQueryResult> items;

	// 結果を取得し、先頭の候補を履歴の実体として取得する
	if (req->GetResult(&items) && items->IsEmpty() == false) {

		static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_HISTORY);

		size_t count = items->GetCount();
		for (size_t i = 0; i < count; ++i) {
			auto cmd = items->GetItem(i);
			if (cmd->GetTypeDisplayName() == TEXT_TYPE) {
				// 履歴コマンドそのものは除外する

				// GetItemで取得したものは参照カウントが増えているので、使わない場合は元に戻す
				cmd->Release();
				continue;
			}

			// キャッシュに登録
			in->mCacheMap.emplace(keyword, CACHE_ITEM{ RefPtr<Command>(cmd, true), GetTickCount64() });
			spdlog::debug(_T("history : cache registered+ {}"), (LPCTSTR)keyword);

			*command = cmd; 
			return true;
		}
	}

	// キーワードに対応するコマンドがみつからなかったものはその旨を覚えておく
	in->mUnavailableKeywordMap[keyword] = GetTickCount64();
	spdlog::debug(_T("history : cache registered- {}"), (LPCTSTR)keyword);
	return false;
}



}}} // end of namespace launcherapp::commands::history

