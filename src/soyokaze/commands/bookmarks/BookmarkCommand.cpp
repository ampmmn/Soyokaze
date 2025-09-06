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
#include "utility/Path.h"
#include "resource.h"
#include <assert.h>

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
	void QueryChromeBookmarks(Pattern* pattern, CommandQueryItemList& commands);
	void QueryEdgeBookmarks(Pattern* pattern, CommandQueryItemList& commands);

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

	// 別スレッドでブックマークのロードを実行する
	std::thread th([&]() {
		auto chromeBookmarks = GetChromeBookmarks();
		if (chromeBookmarks) {
			Path chromeFilePath{Path::USERPROFILE, CHROME_BOOKMARK_PATH};
			chromeBookmarks->Initialize(chromeFilePath);
		}
		auto edgeBookmarks = GetEdgeBookmarks();
		if (edgeBookmarks) {
			Path edgeFilePath{Path::USERPROFILE, EDGE_BOOKMARK_PATH};
			edgeBookmarks->Initialize(edgeFilePath);
		}
	});
	mLoadThread.swap(th);
}

void BookmarkCommand::PImpl::QueryChromeBookmarks(Pattern* pattern, CommandQueryItemList& commands)
{
	auto bookmarks = GetChromeBookmarks();
	if (bookmarks == nullptr) {
		return;
	}

	// 指定されたキーワードでブックマークの検索を行う
	std::vector<Bookmark> bkmItems;
 	bookmarks->Query(pattern, bkmItems, mParam.mIsUseURL);
	for (auto& item : bkmItems) {

		// コマンド名がマッチしているので少なくとも前方一致扱いとする
		int matchLevel = item.mMatchLevel;
		if (matchLevel == Pattern::PartialMatch) {
			matchLevel = Pattern::FrontMatch;
		}
		commands.Add(CommandQueryItem(matchLevel, new URLCommand(item, BrowserType::Chrome)));
	}
}

void BookmarkCommand::PImpl::QueryEdgeBookmarks(Pattern* pattern, CommandQueryItemList& commands)
{
	auto bookmarks = GetEdgeBookmarks();
	if (bookmarks == nullptr) {
		return;
	}

	// 指定されたキーワードでブックマークの検索を行う
	std::vector<Bookmark> bkmItems;
	bookmarks->Query(pattern, bkmItems, mParam.mIsUseURL);
	for (auto& item : bkmItems) {

		// コマンド名がマッチしているので少なくとも前方一致扱いとする
		int matchLevel = item.mMatchLevel;
		if (matchLevel == Pattern::PartialMatch) {
			matchLevel = Pattern::FrontMatch;
		}
		commands.Add(CommandQueryItem(matchLevel, new URLCommand(item, BrowserType::Edge)));
	}
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
	// 終了フラグを立ててスレッド完了を待つ
	if (in->mLoadThread.joinable()) {
		in->mLoadThread.join();
	}
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
	in->mParam.Save(entry);

	return true;
}

bool BookmarkCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr !=GetType()) {
		return false;
	}
	in->mParam.Load(entry);

	in->LoadBookmarks();

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
	command->in->LoadBookmarks();

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

	// Chromeのブックマークを取得し、キーワードで絞り込み
	in->QueryChromeBookmarks(pattern, commands);

	// Edgeのブックマーク一覧を取得し、キーワードで絞り込み
	in->QueryEdgeBookmarks(pattern, commands);

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
	in->LoadBookmarks();
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
	command->in->LoadBookmarks();

	if (newCmd) {
		*newCmd = command.release();
	}
	return true;
}

}}} // end of namespace launcherapp::commands::bookmarks

