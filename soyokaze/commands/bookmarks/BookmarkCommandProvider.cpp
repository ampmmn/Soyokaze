#include "pch.h"
#include "BookmarkCommandProvider.h"
#include "commands/bookmarks/BookmarkCommand.h"
#include "commands/bookmarks/URLCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <list>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace bookmarks {

using BookmarkCommandPtr = std::unique_ptr<BookmarkCommand, std::function<void(void*)> >;
using BookmarkCommandList = std::vector<BookmarkCommandPtr>;

using CommandRepository = launcherapp::core::CommandRepository;

struct BookmarkCommandProvider::PImpl : public launcherapp::core::CommandRepositoryListenerIF
{
	PImpl()
	{
		auto cmdRepo = CommandRepository::GetInstance();
		cmdRepo->RegisterListener(this);

	}
	virtual ~PImpl()
	{
		auto cmdRepo = CommandRepository::GetInstance();
		cmdRepo->UnregisterListener(this);
	}

	void OnDeleteCommand(launcherapp::core::Command* cmd) override
 	{
		for (auto it = mCommands.begin(); it != mCommands.end(); ++it) {
			if ((*it).get() != cmd) {
				continue;
			}

			mCommands.erase(it);
			break;
		}
	}
	void OnLancuherActivate() override {}
	void OnLancuherUnactivate() override {}


	BookmarkCommandList mCommands;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(BookmarkCommandProvider)


BookmarkCommandProvider::BookmarkCommandProvider() : in(std::make_unique<PImpl>())
{
}

BookmarkCommandProvider::~BookmarkCommandProvider()
{
}

static void releaseCmd(void* p)
{
	auto ptr = (BookmarkCommand*)p;
	if (ptr) {
		ptr->Release();
	}
}

CString BookmarkCommandProvider::GetName()
{
	return _T("BookmarkCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString BookmarkCommandProvider::GetDisplayName()
{
	return _T("ブックマーク検索コマンド");
}

// コマンドの種類の説明を示す文字列を取得
CString BookmarkCommandProvider::GetDescription()
{
	static LPCTSTR description = _T("ブラウザのブックマークを検索するコマンドです\n")
		_T("「コマンド名 (キーワード)」でキーワードを含むブックマークを候補として表示できます\n")
		_T("EdgeとChromeに対応しています");
	return description;
}

// コマンド新規作成ダイアログ
bool BookmarkCommandProvider::NewDialog(const CommandParameter* param)
{
	std::unique_ptr<BookmarkCommand> newCmd;
	if (BookmarkCommand::NewDialog(param, newCmd) == false) {
		return false;
	}

	newCmd->AddRef();  // mCommandsで保持する分の参照カウント+1
	in->mCommands.push_back(BookmarkCommandPtr(newCmd.get(), releaseCmd));

	bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void BookmarkCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
)
{
	// 完全一致検索の場合は検索ワード補完をしない
	if (pattern->shouldWholeMatch()) {
		return;
	}

	std::vector<Bookmark> bookmarks;

	CString url;
	CString displayName;
	for (auto& cmd : in->mCommands) {

		if (cmd->QueryBookmarks(pattern, bookmarks) == false) {
			continue;
		}

		for (auto& bkm : bookmarks) {

			LPCTSTR brwoserType = bkm.mBrowser == BrowserType::Chrome ? _T("Chrome") : _T("Edge");

			commands.push_back(CommandQueryItem(bkm.mMatchLevel, 
			                                    new URLCommand(brwoserType, bkm.mName, bkm.mUrl)));
		}
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t BookmarkCommandProvider::GetOrder() const
{
	return 145;
}

bool BookmarkCommandProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<BookmarkCommand> command(new BookmarkCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.get();

	command->AddRef();
	in->mCommands.push_back(BookmarkCommandPtr(command.release(), releaseCmd));

	return true;
}


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

