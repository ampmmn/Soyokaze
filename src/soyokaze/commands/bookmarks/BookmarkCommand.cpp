#include "pch.h"
#include "BookmarkCommand.h"
#include "commands/bookmarks/BookmarkCommandParam.h"
#include "commands/bookmarks/URLCommand.h"
#include "commands/bookmarks/Bookmarks.h"
#include "commands/core/CommandRepository.h"
#include "externaltool/webbrowser/EdgeEnvironment.h"
#include "externaltool/webbrowser/ConfiguredBrowserEnvironment.h"
#include "setting/AppPreference.h"
#include "utility/Path.h"

using namespace launcherapp::commands::common;
using namespace launcherapp::core;
using BrowserEnvironment = launcherapp::externaltool::webbrowser::BrowserEnvironment;
using EdgeEnvironment = launcherapp::externaltool::webbrowser::EdgeEnvironment;
using ConfiguredBrowserEnvironment = launcherapp::externaltool::webbrowser::ConfiguredBrowserEnvironment;

namespace launcherapp { namespace commands { namespace bookmarks {

struct BookmarkCommand::PImpl
{
	void LoadBookmarks();

	bool QueryBookmarks(BrowserEnvironment* brwsEnv, Bookmarks* bookmarks, Pattern* pattern, CommandQueryItemList& commands);

	Bookmarks* GetEdgeBookmarks()
	{
		if (mParam.mIsEnableEdge == false) {
		 return nullptr;
		}
		return &mEdgeBookmarks;
	}

	Bookmarks* GetAlternativeBookmarks()
	{
		if (mParam.mIsEnableAltBrowser == false) {
		 return nullptr;
		}
		return &mAltBrowserBookmarks;
	}

	// コマンドパラメータ
	CommandParam mParam;
	// Webブラウザ(外部ツール)のブックマークデータ
	Bookmarks mAltBrowserBookmarks;
	// Edgeのブックマークデータ
	Bookmarks mEdgeBookmarks;
	// ブックマーク読み込みスレッド
	std::thread mLoadThread;
};

void BookmarkCommand::PImpl::LoadBookmarks()
{
	// 前回のロードが実行中のばあいは完了を待つ
	if (mLoadThread.joinable()) {
		mLoadThread.join();
	}

	uint32_t ignoreMask = mParam.mPrefix.IsEmpty() ? 0 : 1;

	// 別スレッドでブックマークのロードを実行する
	std::thread th([&, ignoreMask]() {

		CString bkmPath;

		auto edgeBookmarks = GetEdgeBookmarks();
		if (edgeBookmarks) {
			auto edge = EdgeEnvironment::GetInstance();
			if (edge->GetBookmarkFilePath(bkmPath)) {
				edgeBookmarks->Initialize(bkmPath);
				edgeBookmarks->SetNumOfKeywordShift(ignoreMask);
			}
		}
		auto altBookmarks = GetAlternativeBookmarks();
		if (altBookmarks) {
			auto altBrws = ConfiguredBrowserEnvironment::GetInstance();
			altBrws->Load();
			if (altBrws->GetBookmarkFilePath(bkmPath)) {
				altBookmarks->Initialize(bkmPath);
				altBookmarks->SetNumOfKeywordShift(ignoreMask);
			}
		}
	});
	mLoadThread.swap(th);
}

bool BookmarkCommand::PImpl::QueryBookmarks(
	BrowserEnvironment* brwsEnv,
 	Bookmarks* bookmarks,
 	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (bookmarks == nullptr) {
		// ブックマーク検索設定で該当するブラウザを使う設定になっていない
		return true;
	}
	if (brwsEnv->IsAvailable() == false) {
		// 外部ツール設定でブラウザを使う設定になっていない(または利用できない)
		return false;
	}

	bool hasPrefix = mParam.mPrefix.IsEmpty() == FALSE;

	auto repos = CommandRepository::GetInstance();

	// 指定されたキーワードでブックマークの検索を行う
	std::vector<Bookmark> bkmItems;
	bookmarks->Query(pattern, bkmItems, mParam.mIsUseURL);
	for (auto& item : bkmItems) {

		// 後続の検索要求が来ている場合は打ち切り
		if (repos->HasQueryRequest()) {
			return false;
		}

		// コマンド名がマッチしているので少なくとも前方一致扱いとする
		int matchLevel = item.mMatchLevel;
		if (hasPrefix && matchLevel == Pattern::PartialMatch) {
			matchLevel = Pattern::FrontMatch;
		}
		commands.Add(CommandQueryItem(matchLevel, new URLCommand(item, brwsEnv)));
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


BookmarkCommand::BookmarkCommand() : in(std::make_unique<PImpl>())
{
}

BookmarkCommand::~BookmarkCommand()
{
	// 終了フラグを立ててスレッド完了を待つ
	if (in->mLoadThread.joinable()) {
		in->mLoadThread.join();
	}
}

bool BookmarkCommand::Load()
{
	auto pref = AppPreference::Get();
	in->mParam.Load((Settings&)pref->GetSettings());

	in->LoadBookmarks();
	return true;
}

bool BookmarkCommand::QueryCandidates(Pattern* pattern, launcherapp::CommandQueryItemList& commands)
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

	// Edgeのブックマーク一覧を取得し、キーワードで絞り込み
	if (in->QueryBookmarks(EdgeEnvironment::GetInstance(), in->GetEdgeBookmarks(), pattern, commands) == false) {
		return false;
	}

	// Webブラウザ(外部ツール)のブックマークを取得し、キーワードで絞り込み
	if (in->QueryBookmarks(ConfiguredBrowserEnvironment::GetInstance(), in->GetAlternativeBookmarks(), pattern, commands) == false) {
		return false;
	}

	return true;
}

}}} // end of namespace launcherapp::commands::bookmarks

