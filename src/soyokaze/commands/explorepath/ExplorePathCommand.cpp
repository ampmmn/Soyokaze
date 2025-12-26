#include "pch.h"
#include "framework.h"
#include "ExplorePathCommand.h"
#include "commands/explorepath/ExplorePathExtraActionSettings.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/core/CommandRepository.h"
#include "actions/core/ActionParameter.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "actions/builtin/ShowPropertiesAction.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "actions/builtin/CallbackAction.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include "mainwindow/controller/MainWindowController.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using SelectionBehavior = launcherapp::core::SelectionBehavior;

using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;
using ShowPropertiesAction = launcherapp::actions::builtin::ShowPropertiesAction;
using OpenPathInFilerAction = launcherapp::actions::builtin::OpenPathInFilerAction;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

using ExecuteHistory = launcherapp::commands::common::ExecuteHistory;

using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;

namespace launcherapp {
namespace commands {
namespace explorepath {

struct ExplorePathCommand::PImpl
{
	bool EnterPath();
	CString mWord;
	CString mFullPath;
	CString mCompletionText;
	ExtraActionSettings* mExtraActionSettings{nullptr};
};

bool ExplorePathCommand::PImpl::EnterPath()
{
	// ウインドウを強制的に前面に出す
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	bool isShowToggle = false;
	mainWnd->ActivateWindow(isShowToggle);

	// 入力欄にパス文字列を入力した状態にする
	CString path(mFullPath);
	if (path.Right(1) != _T('\\') && path.Right(1) != _T('/')) {
		path += _T("\\");
	}
	mainWnd->SetText(path);

	// 再検索
	mainWnd->UpdateCandidateRequest();

	return true;
}

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(ExplorePathCommand)

ExplorePathCommand::ExplorePathCommand(const CString& fullPath) : in(std::make_unique<PImpl>())
{
	in->mWord = fullPath;
	in->mFullPath = fullPath;
	this->mDescription = fullPath;
}

ExplorePathCommand::ExplorePathCommand(const CString& displayName, const CString& fullPath) : in(std::make_unique<PImpl>())
{
	in->mWord = _T("    ") + displayName;
	in->mFullPath = fullPath;
	this->mDescription = fullPath;
}

ExplorePathCommand::~ExplorePathCommand()
{
}

void ExplorePathCommand::SetExtraActionSettings(ExtraActionSettings* settings)
{
	in->mExtraActionSettings = settings;
}

void ExplorePathCommand::SetCompletionText(const CString& completion)
{
	in->mCompletionText = completion;
}

CString ExplorePathCommand::GetName()
{
	return in->mWord;
}

CString ExplorePathCommand::GetTypeDisplayName()
{
	return TypeDisplayName(Path::IsDirectory(in->mFullPath));
}

bool ExplorePathCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (PathIsUNC(in->mFullPath) == FALSE && Path::FileExists(in->mFullPath) == FALSE) {
		// コマンドに関連付けられたパスが存在しない場合は何も実行しない
		return false;
	}

	bool shouldRunAsAdmin = (hotkeyAttr == HOTKEY_ATTR(MOD_SHIFT| MOD_CONTROL, VK_RETURN));
	if (hotkeyAttr == HOTKEY_ATTR(0, VK_RETURN) || shouldRunAsAdmin) {
		// 実行 or 管理者権限で実行
		*action = new CallbackAction(_T("実行"), [&, shouldRunAsAdmin](Parameter* param, String* errMsg) -> bool {
			ExecuteAction a(in->mFullPath, _T("$*"));
			if (shouldRunAsAdmin) {
				a.SetRunAsAdmin();
			}
			bool ret = a.Perform(param, errMsg);

			// フルパスを実行履歴に登録する
			ExecuteHistory::GetInstance()->Add(_T("history"), in->mFullPath);

			return ret;
		});
		return true;
	}

	if (hotkeyAttr == HOTKEY_ATTR(MOD_SHIFT,VK_RETURN) && Path::IsDirectory(in->mFullPath)) {
		// ランチャーでリンク先に遷移する
		*action = new CallbackAction(_T("フォルダ内要素を列挙"), [&](Parameter*, String*) -> bool {
			return in->EnterPath();
		});
		return true;
	}

	else if (hotkeyAttr == HOTKEY_ATTR(MOD_CONTROL, VK_RETURN)) {
		// パスをファイラーで開く
		*action = new OpenPathInFilerAction(in->mFullPath);
		return true;
	}
	else if (hotkeyAttr == HOTKEY_ATTR(MOD_ALT, VK_RETURN)) {
		// プロパティ表示
		*action = new ShowPropertiesAction(in->mFullPath);
		return true;
	}

	return GetExtraAction(hotkeyAttr, action);


	return false;
}

HICON ExplorePathCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mFullPath);
}

launcherapp::core::Command*
ExplorePathCommand::Clone()
{
	auto clonedObj = make_refptr<ExplorePathCommand>(in->mFullPath);
	clonedObj->in->mWord = in->mWord;
	clonedObj->in->mCompletionText = in->mCompletionText;
	return clonedObj.release();
}

// メニューの項目数を取得する
int ExplorePathCommand::GetMenuItemCount()
{
	return 5;
}

// メニューの表示名を取得する
bool ExplorePathCommand::GetMenuItem(int index, Action** action)
{
	if (index < 0 || 4 < index) {
		return false;
	}

	if (index == 0) {
		return GetAction(HOTKEY_ATTR(0, VK_RETURN), action);
	}
	else if (index == 1) {
		*action = new OpenPathInFilerAction(in->mFullPath);
		return true;
	}
	else if (index == 2)  {
		// 管理者権限で実行
		return GetAction(HOTKEY_ATTR(MOD_SHIFT | MOD_CONTROL, VK_RETURN), action);
	}
	else if (index == 3) {
		// クリップボードにコピー
		*action = new CopyTextAction(in->mFullPath);
		return true;
	}
	else { // if (index == 4)
		// プロパティダイアログを表示
		return GetAction(HOTKEY_ATTR(MOD_ALT, VK_RETURN), action);
	}
}

// 選択された
void ExplorePathCommand::OnSelect(Command*)
{
	// 何もしない
}

// 選択解除された
void ExplorePathCommand::OnUnselect(Command*) 
{
	// 何もしない
}

// 実行後のウインドウを閉じる方法
SelectionBehavior::CloseWindowPolicy
ExplorePathCommand::GetCloseWindowPolicy(uint32_t modifierMask)
{
	return SelectionBehavior::CLOSEWINDOW_ASYNC;
}

// 選択時に入力欄に設定するキーワードとキャレットを設定する
bool ExplorePathCommand::CompleteKeyword(CString& keyword, int& startPos, int& endPos)
{
	if (in->mCompletionText.IsEmpty() == FALSE) {
		keyword = in->mCompletionText;		

		if (Path::IsDirectory(in->mFullPath)) {
			keyword += _T("\\");
		}

		startPos = keyword.GetLength();
		endPos = keyword.GetLength();
		return true;
	}
	else {
		// パス末尾に\がなければ追加
		auto newWord = in->mFullPath;
		if (Path::IsDirectory(newWord)) {
			auto c = newWord.Right(1);
			if (c != _T('\\') && c != _T('/')) {
				newWord += _T('\\');
			}
		}

		// 補完後のキーワードの直後にキャレットを設定する
		startPos = newWord.GetLength();
		endPos = newWord.GetLength();

		keyword = newWord;
		return true;
	}
}

// ホットキー設定の数を取得
int ExplorePathCommand::GetHotKeyCount()
{
	return in->mExtraActionSettings ? in->mExtraActionSettings->GetEntryCount() : 0;
}

// ホットキー設定を取得
bool ExplorePathCommand::GetHotKeyAttribute(int index, HOTKEY_ATTR& hotkeyAttr)
{
	if (in->mExtraActionSettings == nullptr) {
		return false;
	}

	ExtraActionSettings::Entry entry;
	if (in->mExtraActionSettings->GetEntry(index, entry) == false) {
		return false;
	}

	hotkeyAttr = entry.mHotkeyAttr;
	return true;
}

bool ExplorePathCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	if (ifid == IFID_SELECTIONBEHAVIOR) {
		AddRef();
		*cmd = (launcherapp::core::SelectionBehavior*)this;
		return true;
	}
	if (ifid == IFID_EXTRAACTIONHOTKEYSETTINGS) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraActionHotKeySettings*)this;
		return true;
	}
	return false;
}

CString ExplorePathCommand::TypeDisplayName(bool isFolder)
{
	return isFolder? _T("フォルダ") : _T("ファイル");
}

// 追加のアクション取得
bool ExplorePathCommand::GetExtraAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	auto& settings = in->mExtraActionSettings;
	if (settings == nullptr) {
		return false;
	}

	auto& fullPath = in->mFullPath;

	int num_entries = settings->GetEntryCount();
	for (int i = 0; i < num_entries; ++i) {

		ExtraActionSettings::Entry entry;
		if (settings->GetEntry(i, entry) == false) {
			continue;
		}
		if (entry.mHotkeyAttr != hotkeyAttr) {
			continue;
		}

		*action = new CallbackAction(entry.mLabel, [fullPath, entry](Parameter* param, String* errMsg) -> bool {
			
			// 実行対象のコマンドを取得
			auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
			RefPtr<launcherapp::core::Command> command(cmdRepo->QueryAsWholeMatch(entry.mCommand, false));
			if (command.get() == nullptr) {
				if (errMsg) {
					*errMsg = "コマンドが見つかりません";
				}
				return false;
			}

			// 実行コマンドに渡す引数を作る
			RefPtr<ParameterBuilder> inParam(ParameterBuilder::Create(), false);
			inParam->AddArgument(fullPath);

			// コマンドからアクションを取得
			RefPtr<Action> action;
			if (command->GetAction(HOTKEY_ATTR(0, VK_RETURN), &action) == false) {
				spdlog::error("Failed to get action.");
				return false;
			}
			// アクションを実行する
			return action->Perform(inParam.get(), errMsg);
		});

		return true;
	}

	return false;
}


} // end of namespace explorepath
} // end of namespace commands
} // end of namespace launcherapp

