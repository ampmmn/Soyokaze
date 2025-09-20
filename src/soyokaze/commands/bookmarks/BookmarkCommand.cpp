#include "pch.h"
#include "BookmarkCommand.h"
#include "commands/bookmarks/BookmarkCommandParam.h"
#include "commands/bookmarks/URLCommand.h"
#include "commands/bookmarks/Bookmarks.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreference.h"
#include "utility/Path.h"

using namespace launcherapp::commands::common;
using namespace launcherapp::core;

namespace launcherapp { namespace commands { namespace bookmarks {

// Chrome $USERPROFILE/AppData/Local/Google/Chrome/User Data/Default/Bookmarks
constexpr LPCTSTR CHROME_BOOKMARK_PATH = _T("AppData\\Local\\Google\\Chrome\\User Data\\Default\\Bookmarks");
// Edge $USERPROFILE/AppData/Local/Microsoft/Edge/User Data/Default/Bookmarks
constexpr LPCTSTR EDGE_BOOKMARK_PATH = _T("AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Bookmarks");

struct BookmarkCommand::PImpl
{
	void LoadBookmarks();
	bool QueryChromeBookmarks(Pattern* pattern, CommandQueryItemList& commands);
	bool QueryEdgeBookmarks(Pattern* pattern, CommandQueryItemList& commands);

	Bookmarks* GetChromeBookmarks()
	{
		if (mParam.mIsEnableChrome == false) {
		 return nullptr;
		}
		return &mChromeBookmarks;
	}

	Bookmarks* GetEdgeBookmarks()
	{
		if (mParam.mIsEnableEdge == false) {
		 return nullptr;
		}
		return &mEdgeBookmarks;
	}

	// コマンドパラメータ
	CommandParam mParam;
	// Chromeのブックマークデータ
	Bookmarks mChromeBookmarks;
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

	int numOfKeywordShift = mParam.mPrefix.IsEmpty() ? 0 : 1;

	// 別スレッドでブックマークのロードを実行する
	std::thread th([&, numOfKeywordShift]() {
		auto chromeBookmarks = GetChromeBookmarks();
		if (chromeBookmarks) {
			Path chromeFilePath{Path::USERPROFILE, CHROME_BOOKMARK_PATH};
			chromeBookmarks->Initialize(chromeFilePath);
			chromeBookmarks->SetNumOfKeywordShift(numOfKeywordShift);
		}
		auto edgeBookmarks = GetEdgeBookmarks();
		if (edgeBookmarks) {
			Path edgeFilePath{Path::USERPROFILE, EDGE_BOOKMARK_PATH};
			edgeBookmarks->Initialize(edgeFilePath);
			edgeBookmarks->SetNumOfKeywordShift(numOfKeywordShift);
		}
	});
	mLoadThread.swap(th);
}

bool BookmarkCommand::PImpl::QueryChromeBookmarks(Pattern* pattern, CommandQueryItemList& commands)
{
	auto bookmarks = GetChromeBookmarks();
	if (bookmarks == nullptr) {
		return true;
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
		commands.Add(CommandQueryItem(matchLevel, new URLCommand(item, BrowserType::Chrome)));
	}
	return true;
}

bool BookmarkCommand::PImpl::QueryEdgeBookmarks(Pattern* pattern, CommandQueryItemList& commands)
{
	auto bookmarks = GetEdgeBookmarks();
	if (bookmarks == nullptr) {
		return true;
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
		commands.Add(CommandQueryItem(matchLevel, new URLCommand(item, BrowserType::Edge)));
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

	// Chromeのブックマークを取得し、キーワードで絞り込み
	if (in->QueryChromeBookmarks(pattern, commands) == false) {
		return false;
	}

	// Edgeのブックマーク一覧を取得し、キーワードで絞り込み
	if (in->QueryEdgeBookmarks(pattern, commands) == false) {
		return false;
	}

	return true;
}

}}} // end of namespace launcherapp::commands::bookmarks

