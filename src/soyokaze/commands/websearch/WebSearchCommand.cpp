#include "pch.h"
#include "WebSearchCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/websearch/WebSearchAdhocCommand.h"
#include "commands/websearch/WebSearchCommandParam.h"
#include "commands/websearch/WebSearchCommandEditor.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/core/CommandRepository.h"
#include "matcher/PatternInternal.h"
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

namespace launcherapp {
namespace commands {
namespace websearch {

struct WebSearchCommand::PImpl
{
	int BuildSearchUrlString(Pattern* pattern, CString& displayName, CString& url);
	int BuildSearchUrlStringAsShortcut(Pattern* pattern, CString& displayName, CString& url);
	CommandParam mParam;

	HICON mIcon = nullptr;
};

int WebSearchCommand::PImpl::BuildSearchUrlString(Pattern* pattern, CString& displayName, CString& url)
{
	const auto& cmdName = mParam.mName;

	RefPtr<PatternInternal> pat2;
	if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
		return Pattern::Mismatch;
	}


	std::vector<CString> words;
	pat2->GetRawWords(words);

	if (words.size() == 1) {
		// 後続キーワードなし
		return Pattern::Mismatch;
	}

	CString queryStr;
	for (size_t i = 1; i < words.size(); ++i) {
		if (i != 1) {
			queryStr += _T(" ");
		}
		queryStr += words[i];
	}

	// URLを生成
	url = mParam.mURL;
	url.Replace(_T("$*"), queryStr);

	// 表示名
	displayName.Format(_T("%s %s"), (LPCTSTR)cmdName, (LPCTSTR)queryStr);

	return Pattern::FrontMatch;
}

// 「常に検索候補として表示する」としての処理
int WebSearchCommand::PImpl::BuildSearchUrlStringAsShortcut(Pattern* pattern, CString& displayName, CString& url)
{
	RefPtr<PatternInternal> pat2;
	if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
		return Pattern::Mismatch;
	}

	std::vector<CString> words;
	pat2->GetRawWords(words);

	if (words.size() == 0) {
		// 後続キーワードなし
		return Pattern::Mismatch;
	}

	CString queryStr;
	for (size_t i = 0; i < words.size(); ++i) {
		if (i != 0) {
			queryStr += _T(" ");
		}
		queryStr += words[i];
	}

	// URLを生成
	url = mParam.mURL;
	url.Replace(_T("$*"), queryStr);

	displayName = queryStr;

	// 「常に検索候補として表示する」として表示する場合は弱い一致を使う
	return Pattern::WeakMatch;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CString WebSearchCommand::GetType() { return _T("WebSearch"); }

WebSearchCommand::WebSearchCommand() : in(std::make_unique<PImpl>())
{
}

WebSearchCommand::~WebSearchCommand()
{
}

bool WebSearchCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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


CString WebSearchCommand::GetName()
{
	return in->mParam.mName;
}

CString WebSearchCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString WebSearchCommand::GetGuideString()
{
	return _T("Enter:検索を実行");
}

CString WebSearchCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMANDNAME_WEBSEARCH);
	return TEXT_TYPE;
}

BOOL WebSearchCommand::Execute(Parameter* param_)
{
	UNREFERENCED_PARAMETER(param_);

	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

	auto cmdline = GetName();
	cmdline += _T(" ");
	SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);
	return TRUE;
}

HICON WebSearchCommand::GetIcon()
{
	if (in->mParam.mIconData.empty()) {
		return IconLoader::Get()->LoadWebIcon();
	}
	else {
		if (in->mIcon == nullptr) {
			in->mIcon = IconLoader::Get()->LoadIconFromStream(in->mParam.mIconData);
			// mIconの解放はIconLoaderが行うので、ここでは行わない
		}
		return in->mIcon;
	}
}

int WebSearchCommand::Match(Pattern* pattern)
{
	if (pattern->shouldWholeMatch() && pattern->Match(GetName()) == Pattern::WholeMatch) {
		// 内部のコマンド名マッチング用の判定
		return Pattern::WholeMatch;
	}
	else if (pattern->shouldWholeMatch() == false) {

		// 入力欄からの入力で、前方一致するときは候補に出す
		int level = pattern->Match(GetName());
		if (level == Pattern::FrontMatch) {
			return Pattern::FrontMatch;
		}

		if (level == Pattern::WholeMatch) {
			// 後続のキーワードが存在する場合は非表示
			return (pattern->GetWordCount() == 1) ? Pattern::WholeMatch : Pattern::HiddenMatch;
		}
	}
	// 通常はこちら
	return Pattern::Mismatch;
}

bool WebSearchCommand::IsEditable()
{
	return true;
}

bool WebSearchCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
WebSearchCommand::Clone()
{
	auto clonedCmd = make_refptr<WebSearchCommand>();

	clonedCmd->in->mParam = in->mParam;

	return clonedCmd.release();
}

bool WebSearchCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());

	entry->Set(_T("description"), GetDescription());

	entry->Set(_T("URL"), in->mParam.mURL);
	entry->Set(_T("IsEnableShortcut"), in->mParam.mIsEnableShortcut);
	entry->SetBytes(_T("IconData"), (const uint8_t*)in->mParam.mIconData.data(), in->mParam.mIconData.size());

	return true;
}

bool WebSearchCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr !=GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));
	in->mParam.mURL = entry->Get(_T("URL"), _T(""));
	in->mParam.mIsEnableShortcut = entry->Get(_T("IsEnableShortcut"), false);

	size_t len = entry->GetBytesLength(_T("IconData"));
	if (len != CommandEntryIF::NO_ENTRY) {
		in->mParam.mIconData.resize(len);
		entry->GetBytes(_T("IconData"), (uint8_t*)in->mParam.mIconData.data(), len);
	}

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 

	return true;
}


bool WebSearchCommand::NewDialog(
	Parameter* param,
	std::unique_ptr<WebSearchCommand>& newCmd
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
	auto command = std::make_unique<WebSearchCommand>();
	command->in->mParam = cmdEditor->GetParam();

	newCmd = std::move(command);

	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool WebSearchCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
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
bool WebSearchCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_WEBSEARCHCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();
	in->mIcon = nullptr;
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool WebSearchCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_WEBSEARCHCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<WebSearchCommand>();
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
bool WebSearchCommand::QueryCandidates(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	// 完全一致検索の場合は検索ワード補完をしない
	if (pattern->shouldWholeMatch()) {
		return false;
	}

	const auto& cmdName = in->mParam.mName;

	bool isMatchName = (cmdName.CompareNoCase(pattern->GetFirstWord()) == 0);
	if (isMatchName == false && in->mParam.IsEnableShortcutSearch() == false) {
		// 名前が一致せず、かつ、「常に検索候補として表示する」でない場合はなにもしない
		return false;
	}

	CString url;
	CString displayName;

	// コマンド名が前方一致する場合は通常処理を使う
	int level = Pattern::Mismatch;
	if (isMatchName || in->mParam.IsEnableShortcutSearch() == false) {
		level = in->BuildSearchUrlString(pattern, displayName, url);
	}
	else {
		// 「常に検索候補として表示する」としての処理
		level = in->BuildSearchUrlStringAsShortcut(pattern, displayName, url);
	}
	if (level == Pattern::Mismatch) {
		return false;
	}

	commands.Add(CommandQueryItem(level, new WebSearchAdhocCommand(this, displayName, url)));

	return true;
}

/**
 	追加候補を表示するために内部でキャッシュしているものがあれば、それを削除する
*/
void WebSearchCommand::ClearCache()
{
	// なし
}


} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

