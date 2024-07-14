#include "pch.h"
#include "WebHistoryCommand.h"
#include "commands/webhistory/WebHistoryCommandParam.h"
#include "commands/webhistory/WebHistorySettingDialog.h"
#include "commands/webhistory/ChromiumBrowseHistory.h"
#include "commands/core/CommandRepository.h"
#include "utility/LastErrorString.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <assert.h>

using namespace launcherapp::commands::common;
using ChromiumBrowseHistory = launcherapp::commands::webhistory::ChromiumBrowseHistory;

namespace launcherapp {
namespace commands {
namespace webhistory {

struct WebHistoryCommand::PImpl
{
	CommandParam mParam;

	ChromiumBrowseHistory* GetChromeHistory()
	{
		if (mParam.mIsEnableHistoryChrome == false) {
			return nullptr;
		}
		if (mChromeHistory.get() == nullptr) {

			// Chromeブラウザのパスを得る
			TCHAR profilePath[MAX_PATH_NTFS];
			size_t reqLen = 0;
			_tgetenv_s(&reqLen, profilePath, MAX_PATH_NTFS, _T("LOCALAPPDATA"));
			PathAppend(profilePath, _T("Google\\Chrome\\User Data\\Default"));

			mChromeHistory.reset(new ChromiumBrowseHistory(_T("chrome"), profilePath, mParam.mIsUseURL, mParam.mIsUseMigemo));
		}
		return mChromeHistory.get();
	}

	ChromiumBrowseHistory* GetEdgeHistory()
	{
		if (mParam.mIsEnableHistoryEdge == false) {
			return nullptr;
		}
		if (mEdgeHistory.get() == nullptr) {

			// Edgeブラウザのパスを得る
			TCHAR profilePath[MAX_PATH_NTFS];
			size_t reqLen = 0;
			_tgetenv_s(&reqLen, profilePath, MAX_PATH_NTFS, _T("LOCALAPPDATA"));
			PathAppend(profilePath, _T("Microsoft\\Edge\\User Data\\Default"));

			mEdgeHistory.reset(new ChromiumBrowseHistory(_T("edge"), profilePath, mParam.mIsUseURL, mParam.mIsUseMigemo));
		}
		return mEdgeHistory.get();
	}

	std::unique_ptr<ChromiumBrowseHistory> mChromeHistory;
	std::unique_ptr<ChromiumBrowseHistory> mEdgeHistory;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CString WebHistoryCommand::GetType() { return _T("WebHistory"); }

WebHistoryCommand::WebHistoryCommand() : in(std::make_unique<PImpl>())
{
}

WebHistoryCommand::~WebHistoryCommand()
{
}

bool WebHistoryCommand::QueryHistories(Pattern* pattern, std::vector<HISTORY>& histories)
{
	const auto& cmdName = in->mParam.mName;
	// コマンド名が一致しなければ候補を表示しない
	if (cmdName.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return false;
	}

	// patternから検索ワード一覧を得る
	std::vector<Pattern::WORD> words;
	pattern->GetWords(words);

	std::reverse(words.begin(), words.end());

	// コマンド名を除外
	words.pop_back();

	// コマンド設定で指定されたキーワードを追加
	const auto& extraKeyword = in->mParam.mKeyword;
	if (extraKeyword.IsEmpty() == FALSE) {

		int n = 0;
		CString tok = extraKeyword.Tokenize(_T(" "), n);
		while(tok.IsEmpty() == FALSE) {

			words.push_back(Pattern::WORD(tok, Pattern::FixString));
			tok = extraKeyword.Tokenize(_T(" "), n);
		}
	}

	std::vector<HISTORY> tmp;

	// Chromeの履歴を検索する
	auto chrome = in->GetChromeHistory();
	if (chrome) {
		std::vector<ChromiumBrowseHistory::ITEM> items;
		chrome->Query(words, items, in->mParam.mLimit, in->mParam.mTimeout);

		for (auto& item : items) {
			if (item.mTitle.IsEmpty()) {
				continue;
			}
			HISTORY history;
			history.mMatchLevel = Pattern::PartialMatch;
			history.mBrowserName = _T("Chrome");
			history.mDisplayName = item.mTitle;
			history.mUrl = item.mUrl;
			tmp.push_back(history);
		}
	}

	// Edgeの履歴を検索する
	auto edge = in->GetEdgeHistory();
	if (edge) {
		std::vector<ChromiumBrowseHistory::ITEM> items;
		in->mEdgeHistory->Query(words, items, in->mParam.mLimit, in->mParam.mTimeout);

		for (auto& item : items) {
			if (item.mTitle.IsEmpty()) {
				continue;
			}
			HISTORY history;
			history.mMatchLevel = Pattern::PartialMatch;
			history.mBrowserName = _T("Edge");
			history.mDisplayName = item.mTitle;
			history.mUrl = item.mUrl;
			tmp.push_back(history);
		}
	}

	// 結果をコピー
	histories.swap(tmp);

	return histories.size() > 0; 
}

CString WebHistoryCommand::GetName()
{
	return in->mParam.mName;
}

CString WebHistoryCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString WebHistoryCommand::GetGuideString()
{
	return _T("Enter:ページを表示する");
}

CString WebHistoryCommand::GetTypeDisplayName()
{
	return _T("ブラウザ履歴検索");
}

BOOL WebHistoryCommand::Execute(const Parameter& param_)
{
	UNREFERENCED_PARAMETER(param_);

	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

	auto cmdline = GetName();
	cmdline += _T(" ");
	SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);
	return TRUE;
}

HICON WebHistoryCommand::GetIcon()
{
	return IconLoader::Get()->LoadWebIcon();
}

int WebHistoryCommand::Match(Pattern* pattern)
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

bool WebHistoryCommand::IsEditable()
{
	return true;
}

int WebHistoryCommand::EditDialog(const Parameter*)
{
	SettingDialog dlg;
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

bool WebHistoryCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool WebHistoryCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
WebHistoryCommand::Clone()
{
	auto clonedCmd = std::make_unique<WebHistoryCommand>();

	clonedCmd->in->mParam = in->mParam;

	return clonedCmd.release();
}

bool WebHistoryCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("Keyword"), in->mParam.mKeyword);
	entry->Set(_T("IsEnableHistoryEdge"), in->mParam.mIsEnableHistoryEdge);
	entry->Set(_T("IsEnableHistoryChrome"), in->mParam.mIsEnableHistoryChrome);
	entry->Set(_T("Timeout"), in->mParam.mTimeout);
	entry->Set(_T("Limit"), in->mParam.mLimit);
	entry->Set(_T("UseMigemo"), in->mParam.mIsUseMigemo);
	entry->Set(_T("UseURL"), in->mParam.mIsUseURL);

	return true;
}

bool WebHistoryCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr !=GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));
	in->mParam.mKeyword = entry->Get(_T("Keyword"), _T(""));
	in->mParam.mIsEnableHistoryEdge = entry->Get(_T("IsEnableHistoryEdge"), true);
	in->mParam.mIsEnableHistoryChrome = entry->Get(_T("IsEnableHistoryChrome"), true);
	in->mParam.mTimeout = entry->Get(_T("Timeout"), 250);
	in->mParam.mLimit = entry->Get(_T("Limit"), 20);
	in->mParam.mIsUseMigemo = entry->Get(_T("UseMigemo"), true);
	in->mParam.mIsUseURL = entry->Get(_T("UseURL"), false);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr);

	return true;
}


bool WebHistoryCommand::NewDialog(
	const Parameter* param,
	std::unique_ptr<WebHistoryCommand>& newCmd
)
{
	// パラメータ指定には対応していない
	UNREFERENCED_PARAMETER(param);

	// 新規作成ダイアログを表示
	SettingDialog dlg;
	//dlg.SetIcon(IconLoader::Get()->LoadWebIcon());
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = dlg.GetParam();
	auto command = std::make_unique<WebHistoryCommand>();
	command->in->mParam = commandParam;

	newCmd = std::move(command);

	return true;
}

} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

