#include "pch.h"
#include "AlignWindowCommand.h"
#include "commands/align_window/AlignWindowCommandParam.h"
#include "commands/align_window/AlignWindowCommandEditor.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/core/CommandRepository.h"
#include "actions/builtin/CallbackAction.h"
#include "hotkey/CommandHotKeyManager.h"
#include "utility/ScopeAttachThreadInput.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "commands/common/Message.h"
#include <assert.h>
#include <regex>

using namespace launcherapp::core;

namespace launcherapp {
namespace commands {
namespace align_window {

using ExecuteHistory = launcherapp::commands::common::ExecuteHistory;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

struct AlignWindowCommand::PImpl
{
	CommandParam mParam;
};

CString AlignWindowCommand::GetType() { return _T("AlignWindow"); }

CString AlignWindowCommand::TypeDisplayName()
{
	static CString TEXT_TYPE(_T("ウインドウ整列"));
	return TEXT_TYPE;
}


/**
 	コンストラクタ
*/
 AlignWindowCommand::AlignWindowCommand() : in(std::make_unique<PImpl>())
{
}

/**
 	デストラクタ
*/
 AlignWindowCommand::~AlignWindowCommand()
{
}

void  AlignWindowCommand::SetParam(const CommandParam& param)
{
	// 更新前に有効パラメータが存在し，かつ、自動実行を許可する場合は
	// 以前の名前で登録していた、履歴の除外ワードを解除する
	if (in->mParam.mName.IsEmpty() == FALSE && IsAllowAutoExecute()) {
		ExecuteHistory::GetInstance()->RemoveExcludeWord(in->mParam.mName);
	}

	// パラメータを上書き
	in->mParam = param;

	// 更新後に自動実行を許可する場合は履歴の除外ワードを登録する
	// (自動実行したいコマンド名が履歴に含まれると、自動実行を阻害することがあるため)
	if (in->mParam.mName.IsEmpty() == FALSE && IsAllowAutoExecute()) {
		ExecuteHistory::GetInstance()->AddExcludeWord(in->mParam.mName);
	}
}


/**
 	コマンド名を取得する
 	@return コマンド名
*/
CString AlignWindowCommand::GetName()
{
	return in->mParam.mName;
}

/**
 	コマンドの説明文字列を取得する
 	@return 説明文字列
*/
CString AlignWindowCommand::GetDescription()
{
	return in->mParam.mDescription;
}

/**
 	ガイド欄文字列を取得する
 	@return ガイド欄文字列
*/
CString AlignWindowCommand::GetGuideString()
{
	return _T("⏎:ウインドウを整列する");
}

/**
 	コマンド種別を表す文字列を取得する
 	@return コマンド種別
*/
CString AlignWindowCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

/**
 * @brief 直前に前面にでていたウインドウハンドルを得る
 * @return ウインドウハンドルまたはNULL
 */
static HWND GetNextHwnd()
{
	// 初回は自分自身の入力画面のハンドルが来ることを想定
	HWND hwnd = GetForegroundWindow();
	while (hwnd) {
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);

		if (IsWindow(hwnd) == FALSE) {
			break;
		}
		// 最初に見つかった可視状態のウインドウを直前に全面にいたウインドウとみなす
		if (IsWindowVisible(hwnd) == FALSE) {
			continue;
		}
		break;
	}
	return hwnd;
}

bool AlignWindowCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	UNREFERENCED_PARAMETER(modifierFlags);

	*action = new CallbackAction(_T("ウインドウを整列する"), [&](Parameter*,String* errMsg) -> bool {
			HWND prevHwnd{nullptr};
			// 対象のウインドウを整列する
			bool isAlignOK = AlignTarget(prevHwnd, errMsg);
			// 整列後に特定のウインドウを前面に設定する
			bool isForegroundOK = SetForeground(prevHwnd, errMsg);
			return isAlignOK && isForegroundOK;
	});

	return true;
}

// 対象のウインドウを整列
bool AlignWindowCommand::AlignTarget(HWND& prevForegroundHwnd, String* errMsg)
{
	String missingTitles;

	HWND hwndForeground = GetNextHwnd();
	prevForegroundHwnd = hwndForeground; 

	std::vector<HWND> targets;
	for (auto& item : in->mParam.mItems) {

		targets.clear();
		item.FindHwnd(targets);

		if (targets.empty()) {
			if (missingTitles.empty() == false) {
				missingTitles += " / ";
			}
			std::string tmp;
			missingTitles += UTF2UTF(item.mCaptionStr, tmp);
			continue;
		}

		for (auto& hwnd : targets) {
			// ウインドウ位置・表示状態(最大/最小/表示/非表示)を復元する
			const WINDOWPLACEMENT& placement = item.mPlacement;
			SetWindowPlacement(hwnd, &placement);
			// Zオーダーを前面に移動
			if (in->mParam.mIsKeepActiveWindow == FALSE) {
				SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOSENDCHANGING);
			}
		}
	}

	if (in->mParam.mIsNotifyIfWindowNotFound && missingTitles.empty() == false ) {
		if (errMsg) {
			*errMsg = fmt::format("以下のウインドウは見つかりませんでした。\n{0}", missingTitles);
		}
		return false;
	}
	return true;
}

// 対象を前面にセット
bool AlignWindowCommand::SetForeground(HWND prevForegroundHwnd, String* errMsg)
{
	UNREFERENCED_PARAMETER(errMsg);

	ScopeAttachThreadInput scope;
	if (in->mParam.mIsKeepActiveWindow) {
		// 整列前に前面にあったウインドウを保つ
		SetForegroundWindow(prevForegroundHwnd);
		return true;
	}

	// 整列した対象のうち、末尾の要素を前面に出す

	if (in->mParam.mItems.empty()) {
		// 対象がないので何もしない
		return true;
	}

	std::vector<HWND> targets;
	in->mParam.mItems.back().FindHwnd(targets);
	if (targets.empty()) {
		// 対象がないので何もしない
		return true;
	}

	SetForegroundWindow(targets[0]);
	return true;
}

HICON AlignWindowCommand::GetIcon()
{
	//return IconLoader::Get()->GetImageResIcon(-5310);
	return IconLoader::Get()->GetImageResIcon(-5357);
}

int AlignWindowCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool AlignWindowCommand::IsAllowAutoExecute()
{
	return in->mParam.mIsAllowAutoExecute;
}


bool AlignWindowCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
AlignWindowCommand::Clone()
{
	auto clonedCmd = make_refptr<AlignWindowCommand>();

	clonedCmd->SetParam(in->mParam);

	return clonedCmd.release();
}

bool AlignWindowCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());

	return in->mParam.Save(entry);
}

bool AlignWindowCommand::Load(CommandEntryIF* entry)
{
	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != AlignWindowCommand::GetType()) {
		return false;
	}

	CommandParam param;
	if (param.Load(entry) == false) {
		return false;
	}

	SetParam(param);
	return true;
}

bool AlignWindowCommand::NewDialog(
	Parameter* param,
	AlignWindowCommand** newCmdPtr
)
{
	// 新規作成ダイアログを表示
	CString value;
	CommandParam paramTmp;

	if (GetNamedParamString(param, _T("COMMAND"), value)) {
		paramTmp.mName = value;
	}
	if (GetNamedParamString(param, _T("DESCRIPTION"), value)) {
		paramTmp.mDescription = value;
	}

	CommandEditor editor;
	editor.SetParam(paramTmp);
	if (editor.DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<AlignWindowCommand>();
	newCmd->SetParam(editor.GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

bool AlignWindowCommand::LoadFrom(CommandFile* cmdFile, void* e, AlignWindowCommand** newCmdPtr)
{
	UNREFERENCED_PARAMETER(cmdFile);
	ASSERT(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;

	auto command = make_refptr<AlignWindowCommand>();
	if (command->Load(entry) == false) {
		return false;
	}

	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}
	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool AlignWindowCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
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
bool AlignWindowCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_ALIGNWINDOWCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	SetParam(cmdEditor->GetParam());
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool AlignWindowCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_ALIGNWINDOWCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<AlignWindowCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

} // end of namespace align_window
} // end of namespace commands
} // end of namespace launcherapp

