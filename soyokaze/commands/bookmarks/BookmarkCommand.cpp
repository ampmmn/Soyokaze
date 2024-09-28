#include "pch.h"
#include "BookmarkCommand.h"
#include "commands/bookmarks/BookmarkCommandParam.h"
#include "commands/bookmarks/URLCommand.h"
#include "commands/bookmarks/BookmarkSettingDialog.h"
#include "commands/bookmarks/Bookmarks.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <assert.h>

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace bookmarks {

constexpr LPCTSTR TYPENAME = _T("BookmarkCommand");

struct BookmarkCommand::PImpl
{
	void Query(BrowserType type, Pattern* pattern, const std::vector<Bookmark> items, std::vector<Bookmark>& out);


	CommandParam mParam;
	Bookmarks mBookmarks;
};

void BookmarkCommand::PImpl::Query(BrowserType type, Pattern* pattern, const std::vector<Bookmark> items, std::vector<Bookmark>& out)
{
	for (auto& item : items) {

		if (item.mUrl.Find(_T("javascript:")) == 0) {
			// ブックマークレットは対象外
			continue;
		}

		int level = pattern->Match(item.mName, 1);
		if (level == Pattern::Mismatch) {

			if (mParam.mIsUseURL == false) {
				// URLを絞り込みに使わない場合はここではじく
				continue;
			}

			level = pattern->Match(item.mUrl, 1);
			if (level == Pattern::Mismatch) {
				continue;
			}
		}

		Bookmark newItem(item);
		newItem.mMatchLevel = level;
		newItem.mBrowser = type;

		out.push_back(newItem);
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CString BookmarkCommand::GetType() { return _T("Bookmark"); }

BookmarkCommand::BookmarkCommand() : in(std::make_unique<PImpl>())
{
}

BookmarkCommand::~BookmarkCommand()
{
}

bool BookmarkCommand::QueryBookmarks(Pattern* pattern, std::vector<Bookmark>& bookmarks)
{
	const auto& cmdName = in->mParam.mName;
	// コマンド名が一致しなければ候補を表示しない
	if (cmdName.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return false;
	}

	// ToDo: 階層をコマンド設定で指定できるようにする

	std::vector<Bookmark> tmp;

	// Chromeのブックマークを取得し、キーワードで絞り込み
	std::vector<Bookmark> items;
	if (in->mParam.mIsEnableChrome && in->mBookmarks.LoadChromeBookmarks(items)) {
		in->Query(BrowserType::Chrome, pattern, items, tmp);
	}

	// Edgeのブックマーク一覧をしゅとくし、キーワードで絞り込み
	if (in->mParam.mIsEnableEdge && in->mBookmarks.LoadEdgeBookmarks(items)) {
		in->Query(BrowserType::Edge, pattern, items, tmp);
	}

	// 結果をコピー
	bookmarks.swap(tmp);

	return bookmarks.size() > 0; 
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
	return _T("Enter:ページを表示する");
}

/**
 * 種別を表す文字列を取得する
 * @return 文字列
 */
CString BookmarkCommand::GetTypeName()
{
	return TYPENAME;
}

CString BookmarkCommand::GetTypeDisplayName()
{
	return _T("ブックマーク検索");
}

BOOL BookmarkCommand::Execute(const Parameter& param_)
{
	UNREFERENCED_PARAMETER(param_);

	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

	auto cmdline = GetName();
	cmdline += _T(" ");
	SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);
	return TRUE;
}

HICON BookmarkCommand::GetIcon()
{
	return IconLoader::Get()->LoadWebIcon();
}

int BookmarkCommand::Match(Pattern* pattern)
{
	bool isWholeMatch = pattern->shouldWholeMatch();
	if (isWholeMatch && pattern->Match(GetName()) == Pattern::WholeMatch) {
		// 内部のコマンド名マッチング用の判定
		return Pattern::WholeMatch;
	}
	else if (isWholeMatch == false) {

		// 入力欄からの入力で、前方一致するときは候補に出す
		int level = pattern->Match(GetName());
		if (level == Pattern::FrontMatch) {
			return Pattern::FrontMatch;
		}
		if (level == Pattern::WholeMatch && pattern->GetWordCount() == 1) {
			return Pattern::WholeMatch;
		}
	}
	// 通常はこちら
	return Pattern::Mismatch;
}

bool BookmarkCommand::IsEditable()
{
	return true;
}

int BookmarkCommand::EditDialog(HWND parent)
{
	SettingDialog dlg(CWnd::FromHandle(parent));
	auto param = in->mParam;

	dlg.SetParam(param);
	if (dlg.DoModal() != IDOK) {
		return 0;
	}
	in->mParam = dlg.GetParam();

	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	return 0;
}

bool BookmarkCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool BookmarkCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
BookmarkCommand::Clone()
{
	auto clonedCmd = std::make_unique<BookmarkCommand>();

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
	const Parameter* param,
	std::unique_ptr<BookmarkCommand>& newCmd
)
{
	// パラメータ指定には対応していない
	UNREFERENCED_PARAMETER(param);

	// 新規作成ダイアログを表示
	SettingDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = dlg.GetParam();
	auto command = std::make_unique<BookmarkCommand>();
	command->in->mParam = commandParam;

	newCmd = std::move(command);

	return true;
}

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

