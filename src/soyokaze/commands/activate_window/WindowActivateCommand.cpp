#include "pch.h"
#include "WindowActivateCommand.h"
#include "commands/activate_window/WindowActivateCommandParam.h"
#include "commands/activate_window/WindowActivateCommandEditor.h"
#include "commands/common/CommandParameterFunctions.h"
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

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace activate_window {

constexpr int UPDATE_INTERVAL = 5000;

struct WindowActivateCommand::PImpl
{
	HWND FindHwnd();
	CommandParam mParam;

	CString mErrorMsg;

	HWND mCachedHwnd{nullptr};
	uint64_t mLastUpdate{0};
};

HWND WindowActivateCommand::PImpl::FindHwnd()
{
	if (IsWindow(mCachedHwnd) && GetTickCount64() - mLastUpdate < UPDATE_INTERVAL) {
		return mCachedHwnd;
	}
	mCachedHwnd = nullptr;

	HWND hwnd = mParam.FindHwnd();
	if (hwnd) {
		mCachedHwnd = hwnd;
		mLastUpdate = GetTickCount64();
	}
	return mCachedHwnd;
}

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

CString WindowActivateCommand::GetName()
{
	return in->mParam.mName;
}

CString WindowActivateCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString WindowActivateCommand::GetGuideString()
{
	return _T("⏎:ウインドウをアクティブにする");
}

CString WindowActivateCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL WindowActivateCommand::Execute(Parameter* param)
{
	// ここで該当するウインドウを探す
	HWND hwndTarget = in->FindHwnd();

	if (IsWindow(hwndTarget) == FALSE) {
		if (in->mParam.IsNotifyIfWindowNotFound()) {
			launcherapp::commands::common::PopupMessage(_T("指定されたウインドウが見つかりません"));
		}
		return TRUE;
	}
	in->mErrorMsg.Empty();

	ScopeAttachThreadInput scope;

	if (IsWindowVisible(hwndTarget) == FALSE) {
		ShowWindow(hwndTarget, SW_SHOW);
	}

	LONG_PTR style = GetWindowLongPtr(hwndTarget, GWL_STYLE);
	if (GetModifierKeyState(param, MASK_CTRL) != 0 && (style & WS_MAXIMIZE) == 0) {
		// Ctrlキーが押されていたら最大化表示する
		PostMessage(hwndTarget, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	else if (style & WS_MINIMIZE) {
		// 最小化されていたら元に戻す
		PostMessage(hwndTarget, WM_SYSCOMMAND, SC_RESTORE, 0);
	}

	SetForegroundWindow(hwndTarget);
	return TRUE;
}

CString WindowActivateCommand::GetErrorString()
{
	return in->mErrorMsg;
}

HICON WindowActivateCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromHwnd(in->FindHwnd());
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

	clonedCmd->in->mParam = in->mParam;

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
	return in->mParam.Load(entry);
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
	auto commandParam = cmdEditor->GetParam();
	auto newCmd = make_refptr<WindowActivateCommand>();
	newCmd->in->mParam = commandParam;

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
	UNREFERENCED_PARAMETER(param);

	RefPtr<WindowActivateCommandEditor> cmdEditor(new WindowActivateCommandEditor);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = cmdEditor->GetParam();
	auto newCmd = make_refptr<WindowActivateCommand>();
	newCmd->in->mParam = commandParam;

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

	in->mParam = cmdEditor->GetParam();
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
	auto commandParam = cmdEditor->GetParam();
	auto newCmd = make_refptr<WindowActivateCommand>();
	newCmd->in->mParam = commandParam;

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

