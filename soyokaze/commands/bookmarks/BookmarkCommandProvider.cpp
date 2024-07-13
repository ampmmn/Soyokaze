#include "pch.h"
#include "BookmarkCommandProvider.h"
#include "commands/bookmarks/URLCommand.h"
#include "commands/bookmarks/Bookmarks.h"
#include "commands/bookmarks/ChromiumBrowseHistory.h"
#include "commands/bookmarks/AppSettingBookmarkPage.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace bookmarks {

using CommandRepository = launcherapp::core::CommandRepository;

struct BookmarkCommandProvider::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Reload();
	}
	void OnAppExit() override {}

	void Reload()
	{
		auto pref = AppPreference::Get();
		mIsEnableBookmark = pref->IsEnableBookmark();
		mIsUseURL = pref->IsUseURLForBookmarkSearch();
		mTimeout = (DWORD)pref->GetBrowserHistoryTimeout();
		mCandidates = pref->GetBrowserHistoryCandidates();
		bool isUseMigemo = pref->IsUseMigemoForBrowserHistory();
		bool isUseURLHistory = pref->IsUseURLForBrowserHistory();
		if (pref->IsEnableHistoryChrome()) {
			TCHAR profilePath[MAX_PATH_NTFS];
			size_t reqLen = 0;
			_tgetenv_s(&reqLen, profilePath, MAX_PATH_NTFS, _T("LOCALAPPDATA"));
			PathAppend(profilePath, _T("Google\\Chrome\\User Data\\Default"));
			mChromeHistory.reset(new ChromiumBrowseHistory(_T("chrome"), profilePath, isUseURLHistory, isUseMigemo));
		}
		else {
			mChromeHistory.reset();
		}
		if (pref->IsEnableHistoryEdge()) {
			TCHAR profilePath[MAX_PATH_NTFS];
			size_t reqLen = 0;
			_tgetenv_s(&reqLen, profilePath, MAX_PATH_NTFS, _T("LOCALAPPDATA"));
			PathAppend(profilePath, _T("Microsoft\\Edge\\User Data\\Default"));
			mEdgeHistory.reset(new ChromiumBrowseHistory(_T("edge"), profilePath, isUseURLHistory, isUseMigemo));
		}
		else {
			mEdgeHistory.reset();
		}
	}
	Bookmarks mBookmarks;
	std::unique_ptr<ChromiumBrowseHistory> mChromeHistory;
	std::unique_ptr<ChromiumBrowseHistory> mEdgeHistory;
	//
	bool mIsEnableBookmark;
	bool mIsUseURL;

	DWORD mTimeout;
	int mCandidates;

	bool mIsFirstCall;

	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(BookmarkCommandProvider)


BookmarkCommandProvider::BookmarkCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mIsEnableBookmark = true;
	in->mIsUseURL = true;
	in->mIsFirstCall = true;

	std::vector<ITEM> items;
	in->mBookmarks.LoadChromeBookmarks(items);
}

BookmarkCommandProvider::~BookmarkCommandProvider()
{
}

CString BookmarkCommandProvider::GetName()
{
	return _T("BookmarkCommand");
}

// 一時的なコマンドを必要に応じて提供する
void BookmarkCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		in->Reload();
		in->mIsFirstCall = false;
	}

	QueryBookmarks(pattern, commands);
	QueryHistories(pattern, commands);
}

void BookmarkCommandProvider::QueryBookmarks(Pattern* pattern, CommandQueryItemList& commands)
{
	if (in->mIsEnableBookmark == false) {
		return ;
	}

	std::vector<ITEM> items;
	if (in->mBookmarks.LoadChromeBookmarks(items)) {
		for (auto& item : items) {

			if (item.mUrl.Find(_T("javascript:")) == 0) {
				// ブックマークレットは対象外
				continue;
			}

			int level = pattern->Match(item.mName);
			if (level == Pattern::Mismatch) {

				if (in->mIsUseURL == false) {
					// URLを絞り込みに使わない場合はここではじく
					continue;
				}
				level = pattern->Match(item.mUrl);
				if (level == Pattern::Mismatch) {
					continue;
				}
			}
			commands.push_back(CommandQueryItem(level, new URLCommand(_T("Chrome"), URLCommand::BOOKMARK, item.mName, item.mUrl)));
		}
	}
	if (in->mBookmarks.LoadEdgeBookmarks(items)) {
		for (auto& item : items) {

			if (item.mUrl.Find(_T("javascript:")) == 0) {
				// ブックマークレットは対象外
				continue;
			}

			int level = pattern->Match(item.mName);
			if (level == Pattern::Mismatch) {

				if (in->mIsUseURL == false) {
					// URLを絞り込みに使わない場合はここではじく
					continue;
				}

				level = pattern->Match(item.mUrl);
				if (level == Pattern::Mismatch) {
					continue;
				}
			}
			commands.push_back(CommandQueryItem(level, new URLCommand(_T("Edge"), URLCommand::BOOKMARK, item.mName, item.mUrl)));
		}
	}
}

void BookmarkCommandProvider::QueryHistories(Pattern* pattern, CommandQueryItemList& commands)
{
	if (in->mChromeHistory.get()) {
		std::vector<ChromiumBrowseHistory::ITEM> items;
		in->mChromeHistory->Query(pattern, items, in->mCandidates, in->mTimeout);

		for (auto& item : items) {
			commands.push_back(CommandQueryItem(Pattern::PartialMatch, new URLCommand(_T("Chrome"), URLCommand::HISTORY, item.mTitle, item.mUrl)));
		}
	}

	if (in->mEdgeHistory.get()) {
		std::vector<ChromiumBrowseHistory::ITEM> items;
		in->mEdgeHistory->Query(pattern, items, in->mCandidates, in->mTimeout);

		for (auto& item : items) {
			if (item.mTitle.IsEmpty()) {
				continue;
			}
			commands.push_back(CommandQueryItem(Pattern::PartialMatch, new URLCommand(_T("Edge"), URLCommand::HISTORY, item.mTitle, item.mUrl)));
		}
	}

}


/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool BookmarkCommandProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	// 必要に応じて実装する
	pages.push_back(new AppSettingBookmarkPage(parent));
	return true;
}

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

