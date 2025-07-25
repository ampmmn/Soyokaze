#include "pch.h"
#include "AlignWindowCommand.h"
#include "commands/align_window/AlignWindowCommandParam.h"
#include "commands/align_window/AlignWindowCommandEditor.h"
#include "commands/core/CommandRepository.h"
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

/**
 	コマンド(ウインドウ整列処理)を実行する
 	@return TRUE:成功 FALSE:失敗
 	@param[in] param コマンド実行時パラメータ
*/
BOOL AlignWindowCommand::Execute(
	Parameter* param
)
{
	UNREFERENCED_PARAMETER(param);

	CString missingTitles;

	ScopeAttachThreadInput scope;

	HWND hwndForeground = GetNextHwnd();

	std::vector<HWND> targets;
	for (auto& item : in->mParam.mItems) {

		targets.clear();
		item.FindHwnd(targets);

		if (targets.empty()) {
			if (missingTitles.IsEmpty() == FALSE) {
				missingTitles += _T(" / ");
			}
			missingTitles += item.mCaptionStr;
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

	if (in->mParam.mIsNotifyIfWindowNotFound && missingTitles.IsEmpty() == FALSE) {
		CString msg;
		msg.Format(_T("以下のウインドウは見つかりませんでした。\n%s"), (LPCTSTR)missingTitles);
		launcherapp::commands::common::PopupMessage(msg);
	}

	if (in->mParam.mIsKeepActiveWindow) {
		SetForegroundWindow(hwndForeground);
	}
	else {
		if (in->mParam.mItems.size() > 0) {
			targets.clear();
			in->mParam.mItems.back().FindHwnd(targets);
			if (targets.size() > 0) {
				SetForegroundWindow(targets[0]);
			}
		}
	}

	return TRUE;
}

CString AlignWindowCommand::GetErrorString()
{
	return _T("");
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

	clonedCmd->in->mParam = in->mParam;

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
	return in->mParam.Load(entry);
}

bool AlignWindowCommand::NewDialog(
	Parameter* param,
	AlignWindowCommand** newCmdPtr
)
{
	UNREFERENCED_PARAMETER(param);

	// パラメータ指定には対応していない
	// param;

	// 新規作成ダイアログを表示
	CommandEditor editor;
	if (editor.DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = editor.GetParam();
	auto newCmd = make_refptr<AlignWindowCommand>();
	newCmd->in->mParam = commandParam;

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

	in->mParam = cmdEditor->GetParam();
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
	auto commandParam = cmdEditor->GetParam();
	auto newCmd = make_refptr<AlignWindowCommand>();
	newCmd->in->mParam = commandParam;

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

} // end of namespace align_window
} // end of namespace commands
} // end of namespace launcherapp

