#include "pch.h"
#include "BookmarkCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/bookmarks/BookmarkCommandEditor.h"
#include "commands/bookmarks/BookmarkCommandParam.h"
#include "commands/bookmarks/URLCommand.h"
#include "commands/bookmarks/Bookmarks.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "icon/IconLoader.h"
#include "mainwindow/controller/MainWindowController.h"
#include "resource.h"
#include <assert.h>

using namespace launcherapp::commands::common;
using namespace launcherapp::core;

namespace launcherapp {
namespace commands {
namespace bookmarks {

struct BookmarkCommand::PImpl
{
	void Query(BrowserType type, Pattern* pattern, const std::vector<Bookmark>& items, std::vector<Bookmark>& out);
	int MatchBookmarkItem(Pattern* pattern, const Bookmark& item);

	CommandParam mParam;
	Bookmarks mBookmarks;
};


void BookmarkCommand::PImpl::Query(BrowserType type, Pattern* pattern, const std::vector<Bookmark>& items, std::vector<Bookmark>& out)
{
	for (auto& item : items) {

		int level = MatchBookmarkItem(pattern, item);
		if (level == Pattern::Mismatch) {
			continue;
		}

		Bookmark newItem(item);
		newItem.mMatchLevel = level;
		newItem.mBrowser = type;

		out.push_back(newItem);
	}
}

int BookmarkCommand::PImpl::MatchBookmarkItem(Pattern* pattern, const Bookmark& item)
{
	if (item.mUrl.Find(_T("javascript:")) == 0) {
		// ブックマークレットは対象外
		return Pattern::Mismatch;
	}

	// まずは名前で比較
	int level = pattern->Match(item.mName, 1);
	if (level != Pattern::Mismatch) {
		return level;
	}

	// 次にフォルダパスで比較
	if (item.mFolderPath.IsEmpty() == FALSE) {
		level = pattern->Match(item.mFolderPath, 1);
		if (level != Pattern::Mismatch) {
			return level;
		}
	}

	if (mParam.mIsUseURL == false) {
		// URLを絞り込みに使わない場合はここではじく
		return Pattern::Mismatch;
	}
	return pattern->Match(item.mUrl, 1);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CString BookmarkCommand::GetType() { return _T("Bookmark"); }

CString BookmarkCommand::TypeDisplayName()
{
	return _T("ブックマーク検索");
}

BookmarkCommand::BookmarkCommand() : in(std::make_unique<PImpl>())
{
}

BookmarkCommand::~BookmarkCommand()
{
}

bool BookmarkCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (UserCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidateSource*)this;
		return true;
	}
	return false;
}

CString BookmarkCommand::GetName()
{
	return in->mParam.mName;
}

CString BookmarkCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString BookmarkCommand::GetGuideString()
{
	return _T("⏎:ページを表示する");
}

CString BookmarkCommand::GetTypeDisplayName()
{
	return BookmarkCommand::TypeDisplayName();
}

BOOL BookmarkCommand::Execute(Parameter* param_)
{
	UNREFERENCED_PARAMETER(param_);

	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	bool isToggle = false;
	mainWnd->ActivateWindow(isToggle);

	auto cmdline = GetName();
	cmdline += _T(" ");
	mainWnd->SetText(cmdline);
	return TRUE;
}

HICON BookmarkCommand::GetIcon()
{
	return IconLoader::Get()->LoadWebIcon();
}

int BookmarkCommand::Match(Pattern* pattern)
{
	bool shouldWholeMatch = pattern->shouldWholeMatch();
	if (shouldWholeMatch && pattern->Match(GetName()) == Pattern::WholeMatch) {
		// 内部のコマンド名マッチング用の判定(完全一致するかどうか)
		return Pattern::WholeMatch;
	}
	else if (shouldWholeMatch == false) {

		// 入力欄からの入力で、前方一致するときは候補に出す
		int level = pattern->Match(GetName());
		if (level == Pattern::FrontMatch || level == Pattern::PartialMatch) {
			return level;
		}
		if (level == Pattern::WholeMatch) {
			// 後続のキーワードが存在する場合は非表示
			return (pattern->GetWordCount() == 1) ? Pattern::WholeMatch : Pattern::HiddenMatch;
		}
	}
	// 通常はこちら
	return Pattern::Mismatch;
}

bool BookmarkCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
BookmarkCommand::Clone()
{
	auto clonedCmd = make_refptr<BookmarkCommand>();

	clonedCmd->in->mParam = in->mParam;

	return clonedCmd.release();
}

bool BookmarkCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	entry->Set(_T("description"), GetDescription());

	entry->Set(_T("EnableChrome"), (bool)in->mParam.mIsEnableChrome);
	entry->Set(_T("EnableEdge"), (bool)in->mParam.mIsEnableEdge);
	entry->Set(_T("UseURL"), (bool)in->mParam.mIsUseURL);

	// ToDo: フォルダ階層指定対応

	return true;
}

bool BookmarkCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr !=GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));

	in->mParam.mIsEnableChrome = entry->Get(_T("EnableChrome"), true);
	in->mParam.mIsEnableEdge = entry->Get(_T("EnableEdge"), true);
	in->mParam.mIsUseURL = entry->Get(_T("UseURL"), false);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr);

	return true;
}


bool BookmarkCommand::NewDialog(
	Parameter* param,
	std::unique_ptr<BookmarkCommand>& newCmd
)
{
	// パラメータ指定には対応していない
	UNREFERENCED_PARAMETER(param);

	// 新規作成ダイアログを表示
	BookmarkCommandEditor editor;
	if (editor.DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = editor.GetParam();
	auto command = std::make_unique<BookmarkCommand>();
	command->in->mParam = commandParam;

	newCmd = std::move(command);

	return true;
}

bool BookmarkCommand::QueryCandidates(Pattern* pattern, CommandQueryItemList& commands)
{
	// コマンド名が一致しなければ候補を表示しない
	if (in->mParam.mName.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return false;
	}

	// 完全一致検索の場合は検索ワード補完をしない
	if (pattern->shouldWholeMatch()) {
		return false;
	}

	std::vector<Bookmark> bookmarks;

	// Chromeのブックマークを取得し、キーワードで絞り込み
	std::vector<Bookmark> items;
	if (in->mParam.mIsEnableChrome && in->mBookmarks.LoadChromeBookmarks(items)) {
		in->Query(BrowserType::Chrome, pattern, items, bookmarks);
	}

	// Edgeのブックマーク一覧を取得し、キーワードで絞り込み
	if (in->mParam.mIsEnableEdge && in->mBookmarks.LoadEdgeBookmarks(items)) {
		in->Query(BrowserType::Edge, pattern, items, bookmarks);
	}

	for (auto& bkm : bookmarks) {

		// コマンド名がマッチしているので少なくとも前方一致扱いとする
		int matchLevel = bkm.mMatchLevel;
		if (matchLevel == Pattern::PartialMatch) {
			matchLevel = Pattern::FrontMatch;
		}

		commands.Add(CommandQueryItem(matchLevel, new URLCommand(bkm)));
	}

	return true;
}

void BookmarkCommand::ClearCache()
{
}

// コマンドを編集するためのダイアログを作成/取得する
bool BookmarkCommand::CreateEditor(HWND parent, CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new BookmarkCommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool BookmarkCommand::Apply(CommandEditor* editor)
{
	RefPtr<BookmarkCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_BOOKMARKCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}
	in->mParam = cmdEditor->GetParam();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool BookmarkCommand::CreateNewInstanceFrom(CommandEditor* editor, Command** newCmd)
{
	RefPtr<BookmarkCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_BOOKMARKCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = cmdEditor->GetParam();
	auto command = make_refptr<BookmarkCommand>();
	command->in->mParam = commandParam;

	if (newCmd) {
		*newCmd = command.release();
	}
	return true;
}



} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

