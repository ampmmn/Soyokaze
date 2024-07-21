#include "pch.h"
#include "WindowActivateCommand.h"
#include "commands/activate_window/WindowActivateCommandParam.h"
#include "commands/activate_window/WindowActivateSettingDialog.h"
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

namespace launcherapp {
namespace commands {
namespace activate_window {

constexpr int UPDATE_INTERVAL = 5000;

struct WindowActivateCommand::PImpl
{
	HWND FindHwnd();
	CommandParam mParam;

	CString mErrorMsg;

	HWND mCachedHwnd = nullptr;
	DWORD mLastUpdate = 0;
};

HWND WindowActivateCommand::PImpl::FindHwnd()
{
	if (IsWindow(mCachedHwnd) && GetTickCount() - mLastUpdate < UPDATE_INTERVAL) {
		return mCachedHwnd;
	}
	mCachedHwnd = nullptr;

	HWND hwnd = mParam.FindHwnd();
	if (hwnd) {
		mCachedHwnd = hwnd;
		mLastUpdate = GetTickCount();
	}
	return mCachedHwnd;
}

CString WindowActivateCommand::GetType() { return _T("WindowActivate"); }


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
	return _T("Enter:ウインドウをアクティブにする");
}


CString WindowActivateCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMANDNAME_WINDOWACTIVATE);
	return TEXT_TYPE;
}

BOOL WindowActivateCommand::Execute(const Parameter& param)
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
	if (param.GetNamedParamBool(_T("CtrlKeyPressed")) && (style & WS_MAXIMIZE) == 0) {
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
	return pattern->Match(GetName());
}

int WindowActivateCommand::EditDialog(HWND parent)
{
	SettingDialog dlg(CWnd::FromHandle(parent));
	dlg.SetParam(in->mParam);
	if (dlg.DoModal() != IDOK) {
		return 0;
	}

	in->mParam = dlg.GetParam();

	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	return 0;
}

bool WindowActivateCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool WindowActivateCommand::IsPriorityRankEnabled()
{
	// 基本は重みづけをする
	return true;
}

launcherapp::core::Command*
WindowActivateCommand::Clone()
{
	auto clonedCmd = std::make_unique<WindowActivateCommand>();

	clonedCmd->in->mParam = in->mParam;

	return clonedCmd.release();
}

bool WindowActivateCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());

	entry->Set(_T("description"), GetDescription());

	entry->Set(_T("CaptionStr"), in->mParam.mCaptionStr);
	entry->Set(_T("ClassStr"), in->mParam.mClassStr);
	entry->Set(_T("IsUseRegExp"), in->mParam.mIsUseRegExp != FALSE);
	entry->Set(_T("IsNotifyIfWindowNotExist"), in->mParam.mIsNotifyIfWindowNotFound != FALSE);

	return true;
}

bool WindowActivateCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != WindowActivateCommand::GetType()) {
		return false;
	}

	CString name = entry->GetName();
	CString descriptionStr = entry->Get(_T("description"), _T(""));

	CString captionStr = entry->Get(_T("CaptionStr"), _T(""));
	CString classStr = entry->Get(_T("ClassStr"), _T(""));
	BOOL isUseRegExp = entry->Get(_T("IsUseRegExp"), false) ? TRUE : FALSE;
	BOOL isNotify = entry->Get(_T("IsNotifyIfWindowNotExist"), false) ? TRUE : FALSE;

	in->mParam.mName = name;
	in->mParam.mDescription = descriptionStr;
	in->mParam.mCaptionStr = captionStr;
	in->mParam.mClassStr = classStr;
	in->mParam.mIsUseRegExp = isUseRegExp;
	in->mParam.mIsNotifyIfWindowNotFound = isNotify;

	if (in->mParam.BuildRegExp() == false) {
		return false;
	}

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 
	return true;
}

bool WindowActivateCommand::NewDialog(
	const Parameter* param,
	WindowActivateCommand** newCmdPtr
)
{
	// パラメータ指定には対応していない
	UNREFERENCED_PARAMETER(param);

	
	// 新規作成ダイアログを表示
	SettingDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = dlg.GetParam();
	auto newCmd = std::make_unique<WindowActivateCommand>();
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

	auto command = std::make_unique<WindowActivateCommand>();
	if (command->Load(entry) == false) {
		return false;
	}

	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}
	return true;
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

