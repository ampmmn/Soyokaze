#include "pch.h"
#include "WebHistoryCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/webhistory/WebHistoryAdhocCommand.h"
#include "commands/webhistory/WebHistoryCommandParam.h"
#include "commands/webhistory/WebHistoryCommandEditor.h"
#include "commands/webhistory/ChromiumBrowseHistory.h"
#include "commands/core/CommandRepository.h"
#include "matcher/PatternInternal.h"
#include "utility/LastErrorString.h"
#include "utility/Path.h"
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
			Path profilePath;
			size_t reqLen = 0;
			_tgetenv_s(&reqLen, profilePath, profilePath.size(), _T("LOCALAPPDATA"));
			profilePath.Append(_T("Google\\Chrome\\User Data\\Default"));

			mChromeHistory.reset(new ChromiumBrowseHistory(_T("chrome"), (LPCTSTR)profilePath, mParam.mIsUseURL, mParam.mIsUseMigemo));
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
			Path profilePath;
			size_t reqLen = 0;
			_tgetenv_s(&reqLen, profilePath, profilePath.size(), _T("LOCALAPPDATA"));
			profilePath.Append(_T("Microsoft\\Edge\\User Data\\Default"));

			mEdgeHistory.reset(new ChromiumBrowseHistory(_T("edge"), (LPCTSTR)profilePath, mParam.mIsUseURL, mParam.mIsUseMigemo));
		}
		return mEdgeHistory.get();
	}

	void QueryHistory(ChromiumBrowseHistory* historyDB, LPCTSTR appName, const std::vector<PatternInternal::WORD>& words, CommandQueryItemList& commands);


	std::unique_ptr<ChromiumBrowseHistory> mChromeHistory;
	std::unique_ptr<ChromiumBrowseHistory> mEdgeHistory;
};

/**
 	履歴を問い合わせる
 	@param[in]     historyDB 履歴を保持するDB的なもの
 	@param[in]     appName   アプリ名
 	@param[in]     words     検索ワード
 	@param[out]    commands  取得した履歴から生成された候補
*/
void WebHistoryCommand::PImpl::QueryHistory(
	ChromiumBrowseHistory* historyDB,
	LPCTSTR appName,
	const std::vector<PatternInternal::WORD>& words,
	CommandQueryItemList& commands
)
{
	if (historyDB == nullptr) {
		return;
	}

	// 条件に該当するブラウザ履歴項目一覧を取得する
	std::vector<ChromiumBrowseHistory::ITEM> items;
	historyDB->Query(words, items, mParam.mLimit, mParam.mTimeout);

	// 取得した項目を候補として登録する
	for (auto& item : items) {

		if (item.mTitle.IsEmpty()) {
			// タイトルが空の場合は表示しない
			continue;
		}

		HISTORY history;
		history.mMatchLevel = Pattern::PartialMatch;
		history.mBrowserName = appName;
		history.mDisplayName = item.mTitle;
		history.mUrl = item.mUrl;
		commands.Add(CommandQueryItem(history.mMatchLevel, new WebHistoryAdhocCommand(history)));
	}
}


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

bool WebHistoryCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (__super::QueryInterface(ifid, cmd)) {
		return true;
	}
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidateSource*)this;
		return true;
	}
	return false;
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

BOOL WebHistoryCommand::Execute(Parameter* param_)
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

bool WebHistoryCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
WebHistoryCommand::Clone()
{
	auto clonedCmd = make_refptr<WebHistoryCommand>();

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
	Parameter* param,
	std::unique_ptr<WebHistoryCommand>& newCmd
)
{
	// パラメータ指定には対応していない
	UNREFERENCED_PARAMETER(param);

	// 新規作成ダイアログを表示
	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto command = std::make_unique<WebHistoryCommand>();
	command->in->mParam = cmdEditor->GetParam();

	newCmd = std::move(command);

	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool WebHistoryCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new CommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool WebHistoryCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_WEBHISTORYCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool WebHistoryCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_WEBHISTORYCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<WebHistoryCommand>();
	newCmd->in->mParam = cmdEditor->GetParam();

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

/**
 	コマンドの候補として追加表示する項目を取得する
 	@return true:取得成功   false:取得失敗(表示しない)
 	@param[in]  pattern  入力パターン
 	@param[out] commands 表示する候補
*/
bool WebHistoryCommand::QueryCandidates(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	const auto& cmdName = in->mParam.mName;
	// コマンド名が一致しなければ候補を表示しない
	if (cmdName.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return false;
	}

	RefPtr<PatternInternal> pat2;
	if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
		return false;
	}

	// patternから検索ワード一覧を得る
	std::vector<PatternInternal::WORD> words;
	pat2->GetWords(words);

	std::reverse(words.begin(), words.end());

	// コマンド名を除外
	words.pop_back();

	// コマンド設定で指定されたキーワードを追加
	const auto& extraKeyword = in->mParam.mKeyword;
	if (extraKeyword.IsEmpty() == FALSE) {

		int n = 0;
		CString tok = extraKeyword.Tokenize(_T(" "), n);
		while(tok.IsEmpty() == FALSE) {

			words.push_back(PatternInternal::WORD(tok, PatternInternal::FixString));
			tok = extraKeyword.Tokenize(_T(" "), n);
		}
	}

	// Chromeの履歴を検索する
	in->QueryHistory(in->GetChromeHistory(),_T("Chrome"), words, commands); 

	// Edgeの履歴を検索する
	in->QueryHistory(in->GetEdgeHistory(),_T("Edge"), words, commands); 

	return true;
}


/**
 	追加候補を表示するために内部でキャッシュしているものがあれば、それを削除する
*/
void WebHistoryCommand::ClearCache()
{
	// ない
}



} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

