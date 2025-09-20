#include "pch.h"
#include "WebHistoryCommand.h"
#include "commands/webhistory/WebHistoryQueryCancellationToken.h"
#include "commands/webhistory/WebHistoryAdhocCommand.h"
#include "commands/webhistory/WebHistoryCommandParam.h"
#include "commands/webhistory/ChromiumBrowseHistory.h"
#include "matcher/PatternInternal.h"
#include "utility/LastErrorString.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include "mainwindow/controller/MainWindowController.h"
#include "resource.h"
#include <assert.h>

using namespace launcherapp::commands::common;
using ChromiumBrowseHistory = launcherapp::commands::webhistory::ChromiumBrowseHistory;

namespace launcherapp { namespace commands { namespace webhistory {

struct WebHistoryCommand::PImpl
{
	ChromiumBrowseHistory* GetChromeHistory()
	{
		if (mParam.mIsEnableChrome == false) {
			return nullptr;
		}
		return &mChromeHistory;
	}

	ChromiumBrowseHistory* GetEdgeHistory()
	{
		if (mParam.mIsEnableEdge == false) {
			return nullptr;
		}
		return &mEdgeHistory;
	}

	bool LoadHistories()
	{
		// 前回のロードが実行中のばあいは完了を待つ
		if (mLoadThread.joinable()) {
			mLoadThread.join();
		}

		// 別スレッドで履歴データのロードを実行する
		std::thread th([&]() {
				auto chromeHistories = GetChromeHistory();
				if (chromeHistories) {
					Path profilePath;
					size_t reqLen = 0;
					_tgetenv_s(&reqLen, profilePath, profilePath.size(), _T("LOCALAPPDATA"));
					profilePath.Append(_T("Google\\Chrome\\User Data\\Default"));
					chromeHistories->Initialize(_T("chrome"), (LPCTSTR)profilePath, mParam.mIsUseURL, mParam.mIsUseMigemo);
				}
				auto edgeHistories = GetEdgeHistory();
				if (edgeHistories) {
					// Edgeブラウザのパスを得る
					Path profilePath;
					size_t reqLen = 0;
					_tgetenv_s(&reqLen, profilePath, profilePath.size(), _T("LOCALAPPDATA"));
					profilePath.Append(_T("Microsoft\\Edge\\User Data\\Default"));
					edgeHistories->Initialize(_T("edge"), (LPCTSTR)profilePath, mParam.mIsUseURL, mParam.mIsUseMigemo);
				}
		});
		mLoadThread.swap(th);

		return true;
	}

	void QueryHistory(ChromiumBrowseHistory* historyDB, LPCTSTR appName, const std::vector<PatternInternal::WORD>& words, CommandQueryItemList& commands);


	CommandParam mParam;
	ChromiumBrowseHistory mChromeHistory;
	ChromiumBrowseHistory mEdgeHistory;
	QueryCancellationToken mCancelToken;
	// ブックマーク読み込みスレッド
	std::thread mLoadThread;
};

/**
 	履歴を問い合わせる
 	@param[in]     historyDB 履歴を保持するDB的なもの
 	@param[in]     appName   アプリ名
 	@param[in]     words     検索ワード
 	@param[out]    commands  取得した履歴から生成された候補
*/
void WebHistoryCommand::PImpl::QueryHistory(
	ChromiumBrowseHistory* historyDB,
	LPCTSTR appName,
	const std::vector<PatternInternal::WORD>& words,
	CommandQueryItemList& commands
)
{
	if (historyDB == nullptr) {
		return;
	}

	// 条件に該当するブラウザ履歴項目一覧を取得する
	std::vector<ChromiumBrowseHistory::ITEM> items;
	historyDB->Query(words, mParam.mLimit, &mCancelToken, items);

	int matchLevel = mParam.mPrefix.IsEmpty() ? Pattern::PartialMatch : Pattern::FrontMatch;

	// 取得した項目を候補として登録する
	for (auto& item : items) {

		if (item.mTitle.IsEmpty()) {
			// タイトルが空の場合は表示しない
			continue;
		}
		commands.Add(CommandQueryItem(matchLevel,
		                              new WebHistoryAdhocCommand(mParam.mPrefix, HISTORY{appName, item.mTitle, item.mUrl, matchLevel})));
	}
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



WebHistoryCommand::WebHistoryCommand() : in(std::make_unique<PImpl>())
{
}

WebHistoryCommand::~WebHistoryCommand()
{
	if (in->mLoadThread.joinable()) {
		in->mLoadThread.join();
	}
}

bool WebHistoryCommand::Load()
{
	auto pref = AppPreference::Get();
	in->mParam.Load((Settings&)pref->GetSettings());

	in->LoadHistories();

	return true;
}

/**
 	コマンドの候補として追加表示する項目を取得する
 	@return true:取得成功   false:取得失敗(表示しない)
 	@param[in]  pattern  入力パターン
 	@param[out] commands 表示する候補
*/
bool WebHistoryCommand::QueryCandidates(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	// 機能を利用しない場合は抜ける
	if (in->mParam.mIsEnable == false) {
		return false;
	}

	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = in->mParam.mPrefix;
	bool hasPrefix = prefix.IsEmpty() == FALSE;
	if (hasPrefix && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return false;
	}

	// 問い合わせ文字列の長さが閾値を下回る場合は機能を発動しない
	if (_tcslen(pattern->GetWholeString()) < in->mParam.mMinTriggerLength) {
		return false;
	}

	RefPtr<PatternInternal> pat2;
	if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
		return false;
	}

	// patternから検索ワード一覧を得る
	std::vector<PatternInternal::WORD> words;
	pat2->GetWords(words);

	if (hasPrefix) {
		// コマンド名を除外
		std::reverse(words.begin(), words.end());
		words.pop_back();
	}

	// キャンセルチェック処理の状態を初期化
	in->mCancelToken.ResetState();

	// Chromeの履歴を検索する
	in->QueryHistory(in->GetChromeHistory(),_T("Chrome"), words, commands); 

	// Edgeの履歴を検索する
	in->QueryHistory(in->GetEdgeHistory(),_T("Edge"), words, commands); 

	return true;
}

uint32_t WebHistoryCommand::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(WebHistoryAdhocCommand::TypeDisplayName(_T("chrome")));
	return 1;
}

}}} // end of namespace launcherapp::commands::webhistory

