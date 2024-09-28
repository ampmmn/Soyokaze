#include "pch.h"
#include "WebSearchCommand.h"
#include "commands/websearch/WebSearchCommandParam.h"
#include "commands/websearch/WebSearchSettingDialog.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
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

namespace launcherapp {
namespace commands {
namespace websearch {

constexpr LPCTSTR TYPENAME = _T("WebSearchCommand");

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
	// コマンド名が一致しなければ候補を表示しない
	if (cmdName.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return Pattern::Mismatch;
	}

	std::vector<CString> words;
	pattern->GetRawWords(words);

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
	const auto& cmdName = mParam.mName;

	if (cmdName.CompareNoCase(pattern->GetFirstWord()) == 0) {
		// コマンド名が前方一致する場合は通常処理を使う
		return BuildSearchUrlString(pattern, displayName, url);
	}

	std::vector<CString> words;
	pattern->GetRawWords(words);

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


int WebSearchCommand::BuildSearchUrlString(Pattern* pattern, CString& displayName, CString& url)
{
	if (in->mParam.IsEnableShortcutSearch() == false) {
		return in->BuildSearchUrlString(pattern, displayName, url);
	}
	else {
		return in->BuildSearchUrlStringAsShortcut(pattern, displayName, url);
	}
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

/**
 * 種別を表す文字列を取得する
 * @return 文字列
 */
CString WebSearchCommand::GetTypeName()
{
	return TYPENAME;
}

CString WebSearchCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMANDNAME_WEBSEARCH);
	return TEXT_TYPE;
}

BOOL WebSearchCommand::Execute(const Parameter& param_)
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
		if (level == Pattern::WholeMatch && pattern->GetWordCount() == 1) {
			return Pattern::WholeMatch;
		}
	}
	// 通常はこちら
	return Pattern::Mismatch;
}

bool WebSearchCommand::IsEditable()
{
	return true;
}

int WebSearchCommand::EditDialog(HWND parent)
{
	SettingDialog dlg(CWnd::FromHandle(parent));
	dlg.SetIcon(GetIcon());

	auto param = in->mParam;

	dlg.SetParam(param);
	if (dlg.DoModal() != IDOK) {
		return 0;
	}
	in->mParam = dlg.GetParam();

	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	in->mIcon = nullptr;

	return 0;
}

bool WebSearchCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool WebSearchCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
WebSearchCommand::Clone()
{
	auto clonedCmd = std::make_unique<WebSearchCommand>();

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
	entry->Set(_T("IconData"), in->mParam.mIconData);

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
	entry->Get(_T("IconData"), in->mParam.mIconData);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 

	return true;
}


bool WebSearchCommand::NewDialog(
	const Parameter* param,
	std::unique_ptr<WebSearchCommand>& newCmd
)
{
	// パラメータ指定には対応していない
	UNREFERENCED_PARAMETER(param);

	// 新規作成ダイアログを表示
	SettingDialog dlg;
	dlg.SetIcon(IconLoader::Get()->LoadWebIcon());
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = dlg.GetParam();
	auto command = std::make_unique<WebSearchCommand>();
	command->in->mParam = commandParam;

	newCmd = std::move(command);

	return true;
}

} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

