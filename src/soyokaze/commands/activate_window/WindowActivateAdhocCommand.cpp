#include "pch.h"
#include "framework.h"
#include "WindowActivateAdhocCommand.h"
#include "core/IFIDDefine.h"
#include "commands/activate_window/ActivateIndicatorWindow.h"
#include "utility/ScopeAttachThreadInput.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/activate_window/RestoreWindowAction.h"
#include "actions/activate_window/MaximizeWindowAction.h"
#include "actions/activate_window/MinimizeWindowAction.h"
#include "commands/activate_window/TemporaryWindowNameAction.h"
#include "commands/activate_window/CloseWindowActionWrapper.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_window {

using namespace launcherapp::commands::common;
using namespace launcherapp::actions::activate_window;

struct WindowActivateAdhocCommand::PImpl
{
	// 選択を解除
	void Unselect() {
		if (IsWindow(mHwnd) == FALSE || IsWindowVisible(mHwnd) == FALSE) {
			return;
		}

		// 対象を強調表示
		ActivateIndicatorWindow::GetInstance()->Uncover();
	}


	HWND mHwnd{nullptr};
	MenuEventListener* mMenuEventListener{nullptr};
	CString mPrefix;
	bool mIsMinimized{false};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(WindowActivateAdhocCommand)

WindowActivateAdhocCommand::WindowActivateAdhocCommand(
	HWND hwnd,
	LPCTSTR prefix
) : in(std::make_unique<PImpl>())
{
	in->mHwnd = hwnd;

	TCHAR caption[256];
	GetWindowText(hwnd, caption, 256);
	this->mName = caption;
	this->mDescription = caption;
	in->mPrefix = prefix;

	auto style = GetWindowLongPtr(hwnd, GWL_STYLE);
	in->mIsMinimized = (style & WS_MINIMIZE) != 0;
}

WindowActivateAdhocCommand::~WindowActivateAdhocCommand()
{
}

void WindowActivateAdhocCommand::SetListener(MenuEventListener* listener)
{
	in->mMenuEventListener = listener;
}

CString WindowActivateAdhocCommand::GetName()
{
	CString name;
	if (in->mPrefix.IsEmpty() == FALSE) {
		name = in->mPrefix + _T(" ");
	}
	name += __super::GetName();

	if (in->mIsMinimized) {
		name += _T(" (最小化)");
	}

	return name;
}

CString WindowActivateAdhocCommand::GetGuideString()
{
	return _T("⏎:ウインドウ切替え S-⏎:最大化 C-⏎::最小化 S-C-⏎:閉じる");
}

CString WindowActivateAdhocCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

// 修飾キー押下状態に対応した実行アクションを取得する
bool WindowActivateAdhocCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	bool isCtrlKeyPressed = modifierFlags & Command::MODIFIER_CTRL;
	bool isShiftKeyPressed = modifierFlags & Command::MODIFIER_SHIFT;

	if (isShiftKeyPressed && isCtrlKeyPressed == false) {
		// Shiftキーが押されていたら最大化表示する
		*action = new MaximizeWindowAction(in->mHwnd);
		return true;
	}
	else if (isCtrlKeyPressed && isShiftKeyPressed == false) {
		// Ctrlキーが押されていたら最小化表示する
		*action = new MinimizeWindowAction(in->mHwnd);
		return true;
	}
	else if (isCtrlKeyPressed && isShiftKeyPressed) {
		// CtrlキーとShiftキーが同時押されていたらウインドウを閉じる
		*action = new CloseWindowActionWrapper(in->mHwnd, in->mMenuEventListener);
		return true;
	}
	else {
		*action = new RestoreWindowAction(in->mHwnd);
		return true;
	}
}

HICON WindowActivateAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromHwnd(in->mHwnd);
}

launcherapp::core::Command*
WindowActivateAdhocCommand::Clone()
{
	return new WindowActivateAdhocCommand(in->mHwnd, in->mPrefix);
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
bool WindowActivateAdhocCommand::SelectMenuItem(int index, Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	if (index < 0 || 5 < index) {
		return false;
	}

	if (index == 0) {
		RestoreWindowAction action(in->mHwnd);
		return action.Perform(nullptr, nullptr);
	}
	else if (index == 1) {
		MaximizeWindowAction action(in->mHwnd);
		return action.Perform(nullptr, nullptr);
	}
	else if (index == 2) {
		MinimizeWindowAction action(in->mHwnd);
		return action.Perform(nullptr, nullptr);
	}
	else if (index == 3) {
		TemporaryWindowNameAction action(in->mHwnd, in->mMenuEventListener);
		return action.Perform(nullptr, nullptr);
	}
	else { // if (index == 4)
		CloseWindowActionWrapper action(in->mHwnd, in->mMenuEventListener);
		return action.Perform(nullptr, nullptr);
	}
}

// 選択された
void WindowActivateAdhocCommand::OnSelect(Command* prior)
{
	UNREFERENCED_PARAMETER(prior);

	if (IsWindow(in->mHwnd) == FALSE || IsWindowVisible(in->mHwnd) == FALSE) {
		return;
	}

	//対象のウインドウを強調表示する

	// ランチャーウインドウの後ろに表示するため、現在ランチャーウインドウの直後にあるウインドウを取得
	SharedHwnd mainWnd;
	auto hwndPrev = GetWindow(mainWnd.GetHwnd(), GW_HWNDNEXT);

	// SetWindowPosでZOrderを変更すると、WS_EX_TOPMOST属性がつくことがあるので復元できるよう属性を保持する
	bool hasTopMost1 = (GetWindowLongPtr(in->mHwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;

	// ランチャーの直後にあるウインドウの前に対象ウインドウをZOrderのみ移動する
	SetWindowPos(in->mHwnd, hwndPrev, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

	bool hasTopMost2 = (GetWindowLongPtr(in->mHwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
	if (hasTopMost1 != hasTopMost2) {
		// WS_EX_TOPMOST属性をもとに戻す
		SetWindowPos(in->mHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	}

	// 対象を強調表示
	ActivateIndicatorWindow::GetInstance()->Cover(in->mHwnd);
}

// 選択解除された
void WindowActivateAdhocCommand::OnUnselect(Command* next)
{
	UNREFERENCED_PARAMETER(next);

	// OnSelectで強調表示したものをもとに戻す
	in->Unselect();
}

// 実行後のウインドウを閉じる方法を決定する
launcherapp::core::SelectionBehavior::CloseWindowPolicy
WindowActivateAdhocCommand::GetCloseWindowPolicy()
{
	return launcherapp::core::SelectionBehavior::CLOSEWINDOW_SYNC;
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
	if (ifid == IFID_SELECTIONBEHAVIOR) {
		AddRef();
		*cmd = (launcherapp::core::SelectionBehavior*)this;
		return true;
	}
	return false;
}

CString WindowActivateAdhocCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_WINDOWACTIVATE);
	return TEXT_TYPE;
}


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

