#include "pch.h"
#include "LauncherMainWindowController.h"
#include "SharedHwnd.h"

namespace launcherapp { namespace mainwindow { namespace controller {

LauncherMainWindowController::LauncherMainWindowController()
{
}

LauncherMainWindowController::~LauncherMainWindowController()
{
}

// キーが編集されたことを通知する(指定した仮想キーを入力する)
void LauncherMainWindowController::InputKey(int vk)
{
	SharedHwnd sharedHwnd;
	PostMessage(sharedHwnd.GetHwnd(), INPUTKEY, vk, 0);
}

// ウインドウをアクティブにする
void LauncherMainWindowController::ActivateWindow(bool isToggle)
{
	SharedHwnd sharedHwnd;
	PostMessage(sharedHwnd.GetHwnd(), ACTIVATEWINDOW, isToggle ? 0 : 1, 0);
}

// コマンドを実行する
void LauncherMainWindowController::RunCommand(LPCTSTR text, bool isWaitSync)
{
	SharedHwnd sharedHwnd;
	SendMessage(sharedHwnd.GetHwnd(), RUNCOMMAND, isWaitSync ? 1 : 0, (LPARAM)text);
}

// テキストを設定する
void LauncherMainWindowController::SetText(LPCTSTR text)
{
	SharedHwnd sharedHwnd;
	SendMessage(sharedHwnd.GetHwnd(), SETTEXT, 0, (LPARAM)text);
}

// ウインドウを非表示にする
void LauncherMainWindowController::HideWindow()
{
	SharedHwnd sharedWnd;
	::SendMessage(sharedWnd.GetHwnd(), HIDEWINDOW, 0, 0);
}

// アプリケーションを終了する
void LauncherMainWindowController::QuitApplication()
{
	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (hwnd) {
		PostMessage(hwnd, QUITAPPLICATION, 0, 0);
	}
}

// クリップボードにテキストを設定
void LauncherMainWindowController::SetClipboardString(HGLOBAL hMem, BOOL* isSetPtr)
{
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), SETCLIPBOARDSTRING, (WPARAM)isSetPtr, (LPARAM)hMem);
}

// クリップボードからテキストを取得
void LauncherMainWindowController::GetClipboardString(CString& text)
{
	SharedHwnd sharedWnd;
	::SendMessage(sharedWnd.GetHwnd(), GETCLIPBOARDSTRING, 0, (LPARAM)&text);
}

// フォーカスを失ったときに隠れるのを阻害する
void LauncherMainWindowController::BlockDeactivateOnUnfocus(bool isBlock)
{
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), BLOCKDEACTIVATE, 0, isBlock ? 1 : 0);
}

// 候補を更新する依頼を出す(非同期)
void LauncherMainWindowController::UpdateCandidateRequest()
{
	SharedHwnd sharedWnd;
	PostMessage(sharedWnd.GetHwnd(), UPDATECANDIDATE, 0, 0);
}

// 入力欄のテキストをクリップボードにコピーする
void LauncherMainWindowController::CopyInputText()
{
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), COPYINPUTTEXT, 0, 0);
}

// 指定したコールバック関数をメインウインドウ側のスレッドで呼んでもらう
void LauncherMainWindowController::RequestCallback(CALLBACKFUNC func, void* param)
{
	SharedHwnd sharedWnd;
	::SendMessage(sharedWnd.GetHwnd(), WM_APP+17, (WPARAM)func, (LPARAM)param);
}

// 入力欄のテキストを空にする
void LauncherMainWindowController::ClearContent()
{
	SharedHwnd sharedWnd;
	::SendMessage(sharedWnd.GetHwnd(), WM_APP+18, 0, 0);
}

// コンテキストメニューを表示する
void LauncherMainWindowController::ShowContextMenu()
{
	SharedHwnd sharedWnd;
	::SendMessage(sharedWnd.GetHwnd(), WM_CONTEXTMENU, 0, MAKELPARAM(-1, -1));
}

// ウインドウを一時的に移動する
bool LauncherMainWindowController::MoveTemporary(int vk)
{
	SharedHwnd sharedWnd;
	return ::SendMessage(sharedWnd.GetHwnd(), MOVETEMPORARY, (WPARAM)vk, 0) == 0;
}



}}} // end of namespace launcherapp::mainwindow::controller

