#include "pch.h"
#include "framework.h"
#include "ExplorePathCommand.h"
#include "commands/common/CommandParameterFunctions.h"
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

namespace launcherapp {
namespace commands {
namespace explorepath {

struct ExplorePathCommand::PImpl
{
	bool EnterPath();
	CString mWord;
	CString mFullPath;
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

CString ExplorePathCommand::GetName()
{
	return in->mWord;
}

CString ExplorePathCommand::GetTypeDisplayName()
{
	return TypeDisplayName(Path::IsDirectory(in->mFullPath));
}

bool ExplorePathCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (PathIsUNC(in->mFullPath) == FALSE && Path::FileExists(in->mFullPath) == FALSE) {
		// コマンドに関連付けられたパスが存在しない場合は何も実行しない
		return false;
	}

	if (modifierFlags == 0) {
		// 実行
		auto a = new ExecuteAction(in->mFullPath, _T("$*"));
		a->SetHistoryPolicy(ExecuteAction::HISTORY_ALWAYS);
		*action = a;
		return true;
	}

	if (modifierFlags == Command::MODIFIER_SHIFT && Path::IsDirectory(in->mFullPath)) {
		// ランチャーでリンク先に遷移する
		*action = new CallbackAction(_T("フォルダ内要素を列挙"), [&](Parameter*, String*) -> bool {
			return in->EnterPath();
		});
		return true;
	}

	if (modifierFlags == (Command::MODIFIER_SHIFT | Command::MODIFIER_CTRL)) {
		// 管理者権限で実行
		auto a = new ExecuteAction(in->mFullPath, _T("$*"));
		a->SetHistoryPolicy(ExecuteAction::HISTORY_ALWAYS);
		a->SetRunAsAdmin();
		*action = a;
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_CTRL) {
		// パスをファイラーで開く
		*action = new OpenPathInFilerAction(in->mFullPath);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_ALT) {
		// プロパティ表示
		*action = new ShowPropertiesAction(in->mFullPath);
		return true;
	}
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
		return GetAction(0, action);
	}
	else if (index == 1) {
		*action = new OpenPathInFilerAction(in->mFullPath);
		return true;
	}
	else if (index == 2)  {
		// 管理者権限で実行
		auto a = new ExecuteAction(in->mFullPath);
		a->SetHistoryPolicy(ExecuteAction::HISTORY_ALWAYS);
		a->SetRunAsAdmin();
		*action = a;
		return true;
	}
	else if (index == 3) {
		// クリップボードにコピー
		*action = new CopyTextAction(in->mFullPath);
		return true;
	}
	else { // if (index == 4)
				 // プロパティダイアログを表示
		*action = new ShowPropertiesAction(in->mFullPath);
		return true;
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
	auto newWord = in->mFullPath;
	if (Path::IsDirectory(newWord)) {
		auto c = newWord.Right(1);
		if (c != _T('\\') && c != _T('/')) {
			newWord += _T('\\');
		}
	}
	if (in->mFullPath.Find(keyword) == 0) {
		startPos = keyword.GetLength();
		endPos = newWord.GetLength();
	}
	else {
		startPos = newWord.GetLength();
		endPos = newWord.GetLength();
	}

	keyword = newWord;
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
	return false;
}

CString ExplorePathCommand::TypeDisplayName(bool isFolder)
{
	return isFolder? _T("フォルダ") : _T("ファイル");
}


} // end of namespace explorepath
} // end of namespace commands
} // end of namespace launcherapp

