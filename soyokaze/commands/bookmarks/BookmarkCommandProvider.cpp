#include "pch.h"
#include "BookmarkCommandProvider.h"
#include "commands/bookmarks/BookmarkCommand.h"
#include "commands/bookmarks/Bookmarks.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "AppPreferenceListenerIF.h"
#include "AppPreference.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace bookmarks {

using CommandRepository = soyokaze::core::CommandRepository;

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
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsEnableBookmark = pref->IsEnableBookmark();
	}
	void OnAppExit() override {}

	Bookmarks mBookmarks;
	//
	bool mIsEnableBookmark;

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
		auto pref = AppPreference::Get();
		in->mIsEnableBookmark = pref->IsEnableBookmark();
		in->mIsFirstCall = false;
	}

	if (in->mIsEnableBookmark == false) {
		return ;
	}

	std::vector<ITEM> items;
	if (in->mBookmarks.LoadChromeBookmarks(items)) {
		for (auto& item : items) {
			int level = pattern->Match(item.mName);
			if (level == Pattern::Mismatch) {
				level = pattern->Match(item.mUrl);
				if (level == Pattern::Mismatch) {
					continue;
				}
			}
			commands.push_back(CommandQueryItem(level, new BookmarkCommand(_T("Chrome"), item.mName, item.mUrl)));
		}
	}
	if (in->mBookmarks.LoadEdgeBookmarks(items)) {
		for (auto& item : items) {
			int level = pattern->Match(item.mName);
			if (level == Pattern::Mismatch) {
				level = pattern->Match(item.mUrl);
				if (level == Pattern::Mismatch) {
					continue;
				}
			}
			commands.push_back(CommandQueryItem(level, new BookmarkCommand(_T("Edge"), item.mName, item.mUrl)));
		}
	}
}


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace soyokaze

