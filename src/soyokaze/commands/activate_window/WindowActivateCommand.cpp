#include "pch.h"
#include "WindowActivateCommand.h"
#include "commands/activate_window/WindowActivateCommandParam.h"
#include "commands/activate_window/WindowActivateCommandEditor.h"
#include "commands/activate_window/ActivateWindowFindTarget.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/core/CommandRepository.h"
#include "actions/activate_window/MaximizeWindowAction.h"
#include "actions/activate_window/RestoreWindowAction.h"
#include "hotkey/CommandHotKeyManager.h"
#include "utility/ScopeAttachThreadInput.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "commands/common/Message.h"
#include <assert.h>
#include <regex>

using namespace launcherapp::commands::common;
using namespace launcherapp::actions::activate_window;

namespace launcherapp {
namespace commands {
namespace activate_window {

struct WindowActivateCommand::PImpl
{
	CommandParam mParam;
	ActivateWindowFindTarget mTarget;
};

CString WindowActivateCommand::GetType() { return _T("WindowActivate"); }

CString WindowActivateCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMANDNAME_WINDOWACTIVATE);
	return TEXT_TYPE;
}

WindowActivateCommand::WindowActivateCommand() : in(std::make_unique<PImpl>())
{
}

WindowActivateCommand::~WindowActivateCommand()
{
}

void WindowActivateCommand::SetParam(const CommandParam& param)
{
	// 更新前に有効パラメータが存在し，かつ、自動実行を許可する場合は
	// 以前の名前で登録していた、履歴の除外ワードを解除する
	if (in->mParam.mName.IsEmpty() == FALSE && IsAllowAutoExecute()) {
		ExecuteHistory::GetInstance()->RemoveExcludeWord(in->mParam.mName);
	}

	// パラメータを上書き
	in->mParam = param;
	in->mTarget.SetParam(param);

	// 更新後に自動実行を許可する場合は履歴の除外ワードを登録する
	// (自動実行したいコマンド名が履歴に含まれると、自動実行を阻害することがあるため)
	if (in->mParam.mName.IsEmpty() == FALSE && IsAllowAutoExecute()) {
		ExecuteHistory::GetInstance()->AddExcludeWord(in->mParam.mName);
	}
}

CString WindowActivateCommand::GetName()
{
	return in->mParam.mName;
}

CString WindowActivateCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString WindowActivateCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

// 修飾キー押下状態に対応した実行アクションを取得する
bool WindowActivateCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	UNREFERENCED_PARAMETER(action);

	if (modifierFlags == Command::MODIFIER_CTRL) {
		auto action_ = new MaximizeWindowAction(in->mTarget.Clone());
		action_->SetSilent(in->mParam.IsNotifyIfWindowNotFound() == false);
		*action = action_;
		return true;
	}
	else if (modifierFlags == 0) {
		auto action_ = new RestoreWindowAction(in->mTarget.Clone());
		action_->SetSilent(in->mParam.IsNotifyIfWindowNotFound() == false);
		*action = action_;
		return true;
	}
	return false;
}

HICON WindowActivateCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromHwnd(in->mTarget.GetHandle());
}

int WindowActivateCommand::Match(Pattern* pattern)
{
	if (pattern->shouldWholeMatch() == false && in->mParam.mIsHotKeyOnly) {
		// ホットキーからの実行専用の場合は候補に表示させない
		return Pattern::Mismatch;
	}

	return pattern->Match(GetName());
}

bool WindowActivateCommand::IsAllowAutoExecute()
{
	return in->mParam.mIsAllowAutoExecute;
}


bool WindowActivateCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
WindowActivateCommand::Clone()
{
	auto clonedCmd = make_refptr<WindowActivateCommand>();

	clonedCmd->SetParam(in->mParam);

	return clonedCmd.release();
}

bool WindowActivateCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	return in->mParam.Save(entry);
}

bool WindowActivateCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != WindowActivateCommand::GetType()) {
		return false;
	}

	CommandParam param;
	if (param.Load(entry) == false) {
		return false;
	}

	SetParam(param);

	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool WindowActivateCommand::NewInstance(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<WindowActivateCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_WINDOWACTIVATECOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<WindowActivateCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

bool WindowActivateCommand::NewDialog(
	Parameter* param,
	WindowActivateCommand** newCmdPtr
)
{
	// パラメータ指定には対応していない
	CString value;
	CommandParam paramTmp;

	if (GetNamedParamString(param, _T("COMMAND"), value)) {
		paramTmp.mName = value;
	}
	if (GetNamedParamString(param, _T("DESCRIPTION"), value)) {
		paramTmp.mDescription = value;
	}

	RefPtr<WindowActivateCommandEditor> cmdEditor(new WindowActivateCommandEditor);
	cmdEditor->SetParam(paramTmp);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = cmdEditor->GetParam();
	auto newCmd = make_refptr<WindowActivateCommand>();
	newCmd->SetParam(commandParam);

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

bool WindowActivateCommand::LoadFrom(CommandFile* cmdFile, void* e, WindowActivateCommand** newCmdPtr)
{
	UNREFERENCED_PARAMETER(cmdFile);

	ASSERT(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;

	auto command = make_refptr<WindowActivateCommand>();
	if (command->Load(entry) == false) {
		return false;
	}

	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}
	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool WindowActivateCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new WindowActivateCommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool WindowActivateCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<WindowActivateCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_WINDOWACTIVATECOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	SetParam(cmdEditor->GetParam());
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool WindowActivateCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<WindowActivateCommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_WINDOWACTIVATECOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<WindowActivateCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

