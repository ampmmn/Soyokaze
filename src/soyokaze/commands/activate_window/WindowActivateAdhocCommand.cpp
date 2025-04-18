#include "pch.h"
#include "framework.h"
#include "WindowActivateAdhocCommand.h"
#include "commands/core/IFIDDefine.h"
#include "utility/ScopeAttachThreadInput.h"
#include "commands/common/CommandParameterFunctions.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_window {

using namespace launcherapp::commands::common;

struct WindowActivateAdhocCommand::PImpl
{
	bool Maximize();
	bool Minimize();
	bool GiveAdhocName();
	bool Close();

	HWND mHwnd{nullptr};
	MenuEventListener* mMenuEventListener{nullptr};
};

bool WindowActivateAdhocCommand::PImpl::Maximize()
{
	PostMessage(mHwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	SetForegroundWindow(mHwnd);
	return true;
}

bool WindowActivateAdhocCommand::PImpl::Minimize()
{
	ShowWindow(mHwnd, SW_MINIMIZE);
	return true;
}

bool WindowActivateAdhocCommand::PImpl::GiveAdhocName()
{
	if (mMenuEventListener) {
		mMenuEventListener->OnRequestPutName(mHwnd);
	}
	return true;
}

bool WindowActivateAdhocCommand::PImpl::Close()
{
	PostMessage(mHwnd, WM_CLOSE, 0, 0);
	if (mMenuEventListener) {
		mMenuEventListener->OnRequestClose(mHwnd);
	}
	return true;
}


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(WindowActivateAdhocCommand)

WindowActivateAdhocCommand::WindowActivateAdhocCommand(
	HWND hwnd
) : in(std::make_unique<PImpl>())
{
	in->mHwnd = hwnd;

	TCHAR caption[256];
	GetWindowText(hwnd, caption, 256);
	this->mName = caption;
	this->mDescription = caption;
}

WindowActivateAdhocCommand::~WindowActivateAdhocCommand()
{
}

void WindowActivateAdhocCommand::SetListener(MenuEventListener* listener)
{
	in->mMenuEventListener = listener;
}

CString WindowActivateAdhocCommand::GetGuideString()
{
	return _T("Enter:ウインドウをアクティブにする");
}

CString WindowActivateAdhocCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_WINDOWACTIVATE);
	return TEXT_TYPE;
}

BOOL WindowActivateAdhocCommand::Execute(Parameter* param)
{
	ScopeAttachThreadInput scope;

	bool isCtrlKeyPressed = GetModifierKeyState(param, MASK_CTRL) != 0;

	LONG_PTR style = GetWindowLongPtr(in->mHwnd, GWL_STYLE);
	if (isCtrlKeyPressed && (style & WS_MAXIMIZE) == 0) {
		// Ctrlキーが押されていたら最大化表示する
		PostMessage(in->mHwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	else if (style & WS_MINIMIZE) {
		// 最小化されていたら元に戻す
		PostMessage(in->mHwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	}

	SetForegroundWindow(in->mHwnd);
	return TRUE;
}

HICON WindowActivateAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromHwnd(in->mHwnd);
}

launcherapp::core::Command*
WindowActivateAdhocCommand::Clone()
{
	return new WindowActivateAdhocCommand(in->mHwnd);
}

// メニューの項目数を取得する
int WindowActivateAdhocCommand::GetMenuItemCount()
{
	return 5;
}

// メニューの表示名を取得する
bool WindowActivateAdhocCommand::GetMenuItemName(int index, LPCWSTR* displayNamePtr)
{
	if (index == 0) {
		static LPCWSTR name = L"ウインドウ切替(&A)";
		*displayNamePtr= name;
		return true;
	}
	else if (index == 1) {
		static LPCWSTR name = L"最大化(&X)";
		*displayNamePtr= name;
		return true;
	}
	else if (index == 2) {
		static LPCWSTR name = L"最小化(&M)";
		*displayNamePtr= name;
		return true;
	}
	else if (index == 3) {
		static LPCWSTR name = L"ウインドウに一時的な名前を付ける(&T)";
		*displayNamePtr= name;
		return true;
	}
	else if (index == 4) {
		static LPCWSTR name = L"ウインドウを閉じる(&C)";
		*displayNamePtr= name;
		return true;
	}
	return false;
}

// メニュー選択時の処理を実行する
bool WindowActivateAdhocCommand::SelectMenuItem(int index, launcherapp::core::CommandParameter* param)
{
	if (index < 0 || 5 < index) {
		return false;
	}

	if (index == 0) {
		return Execute(param) != FALSE;
	}
	else if (index == 1) {
		return in->Maximize();
	}
	else if (index == 2) {
		return in->Minimize();
	}
	else if (index == 3) {
		return in->GiveAdhocName();
	}
	else { // if (index == 4)
		return in->Close();
	}
}

bool WindowActivateAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	return false;
}


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

