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


BookmarkCommandProvider::BookmarkCommandProvider() : in(new PImpl)
{
	in->mRefCount = 1;
	in->mIsEnableBookmark = false;
	in->mIsFirstCall = true;
}

BookmarkCommandProvider::~BookmarkCommandProvider()
{
}

// 初回起動の初期化を行う
void BookmarkCommandProvider::OnFirstBoot()
{
	// 何もしない
}


// コマンドの読み込み
void BookmarkCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	// 何もしない
}

CString BookmarkCommandProvider::GetName()
{
	return _T("BookmarkCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString BookmarkCommandProvider::GetDisplayName()
{
	// サポートしない
	return _T("");
}

// コマンドの種類の説明を示す文字列を取得
CString BookmarkCommandProvider::GetDescription()
{
	// サポートしない
	return _T("");
}

// コマンド新規作成ダイアログ
bool BookmarkCommandProvider::NewDialog(const CommandParameter* param)
{
	// サポートしない
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool BookmarkCommandProvider::IsPrivate() const
{
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void BookmarkCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
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
		for (auto item : items) {
			int level = pattern->Match(item.mName);
			if (level == Pattern::Mismatch) {
				continue;
			}
			commands.push_back(CommandQueryItem(level, new BookmarkCommand(_T("Chrome"), item.mName, item.mUrl)));
		}
	}
	if (in->mBookmarks.LoadEdgeBookmarks(items)) {
		for (auto item : items) {
			int level = pattern->Match(item.mName);
			if (level == Pattern::Mismatch) {
				continue;
			}
			commands.push_back(CommandQueryItem(level, new BookmarkCommand(_T("Edge"), item.mName, item.mUrl)));
		}
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t BookmarkCommandProvider::BookmarkCommandProvider::GetOrder() const
{
	return 2000;
}

uint32_t BookmarkCommandProvider::BookmarkCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t BookmarkCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace soyokaze

