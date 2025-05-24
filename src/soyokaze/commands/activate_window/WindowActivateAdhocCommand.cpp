#include "pch.h"
#include "framework.h"
#include "WindowActivateAdhocCommand.h"
#include "commands/core/IFIDDefine.h"
#include "utility/ScopeAttachThreadInput.h"
#include "commands/common/CommandParameterFunctions.h"
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

struct WindowActivateAdhocCommand::PImpl
{
	bool Maximize();
	bool Minimize();
	bool GiveAdhocName();
	bool Close();

	// 選択を解除
	void Unselect() {
		if (IsWindow(mHwnd) == FALSE || IsWindowVisible(mHwnd) == FALSE) {
			return;
		}

		// OnSelectで強調したウインドウをもとに戻す
		SetWindowPos(mHwnd, mPrevZOrder, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		RedrawWindow(mHwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ALLCHILDREN | RDW_ERASE);

		mPrevZOrder = nullptr;
	}


	HWND mHwnd{nullptr};
	HWND mPrevZOrder{nullptr};
	MenuEventListener* mMenuEventListener{nullptr};
	CString mPrefix;
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
	return name;
}

CString WindowActivateAdhocCommand::GetGuideString()
{
	return _T("⏎:ウインドウをアクティブにする");
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

	in->mPrevZOrder = nullptr;
	return TRUE;
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

// 選択された
void WindowActivateAdhocCommand::OnSelect(Command* prior)
{
	UNREFERENCED_PARAMETER(prior);

	if (IsWindow(in->mHwnd) == FALSE || IsWindowVisible(in->mHwnd) == FALSE) {
		return;
	}

	//対象のウインドウを強調表示する

	// ランチャーウインドウの後ろに表示するため、現在ランチャーウインドウの直後にあるウインドウを取得
	in->mPrevZOrder = GetWindow(in->mHwnd, GW_HWNDPREV);

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

	// 枠を強調表示するためクライアント領域をGetWindowDCの座標系で求める
	CRect rcWindow;
	GetWindowRect(in->mHwnd, &rcWindow);
	CRect rc;
	GetClientRect(in->mHwnd, &rc);

	CRect rcClientScrren(rc);
	MapWindowPoints(in->mHwnd, nullptr, (POINT*)&rcClientScrren, 2);

	rc.OffsetRect(rcClientScrren.TopLeft() - rcWindow.TopLeft());
	rc.DeflateRect(2, 2, 3, 3);

	// 枠を描画
	HDC dc = GetWindowDC(in->mHwnd);

	HPEN pen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
	HBRUSH br = (HBRUSH)GetStockObject(NULL_BRUSH);
	auto oldPen = SelectObject(dc, pen);
	auto oldBr = SelectObject(dc, br);

	Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);

	// 後始末
	SelectObject(dc, oldBr);
	SelectObject(dc, oldPen);
	ReleaseDC(in->mHwnd, dc);

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


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

