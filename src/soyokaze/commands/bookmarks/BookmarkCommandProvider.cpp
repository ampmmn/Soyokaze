#include "pch.h"
#include "BookmarkCommandProvider.h"
#include "commands/bookmarks/BookmarkCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace bookmarks {

using CommandRepository = launcherapp::core::CommandRepository;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(BookmarkCommandProvider)


BookmarkCommandProvider::BookmarkCommandProvider()
{
}

BookmarkCommandProvider::~BookmarkCommandProvider()
{
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
bool BookmarkCommandProvider::NewDialog(CommandParameter* param)
{
	std::unique_ptr<BookmarkCommand> newCmd;
	if (BookmarkCommand::NewDialog(param, newCmd) == false) {
		return false;
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t BookmarkCommandProvider::GetOrder() const
{
	return 145;
}

void BookmarkCommandProvider::OnBeforeLoad()
{
}

bool BookmarkCommandProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<BookmarkCommand> command(new BookmarkCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.release();

	return true;
}


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

