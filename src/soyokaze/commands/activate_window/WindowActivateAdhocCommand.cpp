#include "pch.h"
#include "framework.h"
#include "WindowActivateAdhocCommand.h"
#include "core/IFIDDefine.h"
#include "commands/activate_window/ActivateIndicatorWindow.h"
#include "commands/activate_window/WindowActivateCommandParam.h"
#include "utility/ScopeAttachThreadInput.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/activate_window/RestoreWindowAction.h"
#include "actions/activate_window/MaximizeWindowAction.h"
#include "actions/activate_window/MinimizeWindowAction.h"
#include "actions/activate_window/CloseWindowAction.h"
#include "actions/builtin/CallbackAction.h"
#include "commands/activate_window/RegionIndicatorWindow.h"
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
using CallbackAction = launcherapp::actions::builtin::CallbackAction;


struct WindowActivateAdhocCommand::PImpl
{
	void Unselect() {
	// 選択を解除
		ActivateIndicatorWindow::GetInstance()->Uncover();
		RegionIndicatorWindow::GetInstance()->Uncover();
	}


	HWND mHwnd{nullptr};
	CString mCaption;
	CommandParam mParam;
	bool mIsMinimized{false};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(WindowActivateAdhocCommand)

WindowActivateAdhocCommand::WindowActivateAdhocCommand(
	HWND hwnd,
	const CommandParam& param
) : in(std::make_unique<PImpl>())
{
	in->mHwnd = hwnd;
	in->mParam = param;

	TCHAR caption[256];
	GetWindowText(hwnd, caption, 256);
	in->mCaption = caption;

	this->mDescription = caption;

	auto style = GetWindowLongPtr(hwnd, GWL_STYLE);
	in->mIsMinimized = (style & WS_MINIMIZE) != 0;
}

WindowActivateAdhocCommand::~WindowActivateAdhocCommand()
{
}

CString WindowActivateAdhocCommand::GetName()
{
	CString name(in->mParam.mName);
	name += _T(" ");
	name += in->mCaption;

	if (in->mIsMinimized) {
		name += _T(" (最小化)");
	}

	return name;
}

CString WindowActivateAdhocCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

// 修飾キー押下状態に対応した実行アクションを取得する
bool WindowActivateAdhocCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	auto modifierFlags = hotkeyAttr.GetModifiers();

	bool isCtrlKeyPressed = modifierFlags & MOD_CONTROL;
	bool isShiftKeyPressed = modifierFlags & MOD_SHIFT;

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
		*action = new CloseWindowAction(in->mHwnd);
		return true;
	}
	else if (modifierFlags == 0) {

		if (in->mParam.mShouldArrangeWindow == false) {
			*action = new RestoreWindowAction(in->mHwnd);
		}
		else {
			*action = new CallbackAction(_T("ウインドウを配置する"), [&](Parameter*,String*) -> bool {

				ScopeAttachThreadInput scope;
				auto& wp = in->mParam.mPlacement;
				SetWindowPlacement(in->mHwnd, &wp);
				// Zオーダーを前面に移動
				if (in->mParam.mShouldActivateWindow) {
					SetForegroundWindow(in->mHwnd);
				}
				return true;
			});
		}
		return true;
	}
	return false;
}

HICON WindowActivateAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromHwnd(in->mHwnd);
}

launcherapp::core::Command*
WindowActivateAdhocCommand::Clone()
{
	auto cmd = new WindowActivateAdhocCommand(in->mHwnd, in->mParam);

	return cmd;
}

// メニューの項目数を取得する
int WindowActivateAdhocCommand::GetMenuItemCount()
{
	return 4;
}

// メニューの表示名を取得する
bool WindowActivateAdhocCommand::GetMenuItem(int index, Action** action)
{
	if (index < 0 || 3 < index) {
		return false;
	}

	if (index == 0) {
		*action = new RestoreWindowAction(in->mHwnd);
		return true;
	}
	else if (index == 1) {
		*action = new MaximizeWindowAction(in->mHwnd);
		return true;
	}
	else if (index == 2) {
		*action = new MinimizeWindowAction(in->mHwnd);
		return true;
	}
	else { // if (index == 3)
		*action = new CloseWindowAction(in->mHwnd);
		return true;
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
	RegionIndicatorWindow::GetInstance()->Cover(in->mParam.mPlacement.rcNormalPosition);
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
WindowActivateAdhocCommand::GetCloseWindowPolicy(uint32_t modifierFlags)
{
	if (modifierFlags == (MOD_CONTROL | MOD_SHIFT)) {
		return launcherapp::core::SelectionBehavior::CLOSEWINDOW_NOCLOSE;
	}
	else {
		return launcherapp::core::SelectionBehavior::CLOSEWINDOW_SYNC;
	}
}

// 選択時に入力欄に設定するキーワードとキャレットを設定する
bool WindowActivateAdhocCommand::CompleteKeyword(CString& , int& , int& )
{
	return false;
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

