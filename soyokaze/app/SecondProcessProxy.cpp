#include "pch.h"
#include "SecondProcessProxy.h"
#include "SharedHwnd.h"
#include "resource.h"
#include "mainwindow/LauncherMainWindow.h"

namespace launcherapp {

SecondProcessProxy::SecondProcessProxy()
{
}

SecondProcessProxy::~SecondProcessProxy()
{
}

/**
 *  先行プロセスに対しコマンド文字列を送る
 *  (先行プロセス側でコマンドを実行する)
 */
bool SecondProcessProxy::SendCommandString(const CString& commandStr, bool isPasteOnly)
{
	SPDLOG_DEBUG(_T("args commandStr:{0}"), (LPCTSTR)commandStr);

	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (hwnd == NULL) {
		return false;
	}

	HWND hwndCommand = GetDlgItem(hwnd, IDC_EDIT_COMMAND2);
	if (hwndCommand == NULL) {
		SPDLOG_ERROR(_T("CmdReceiveEdit does not found."));
		return false;
	}

	if (isPasteOnly) {
		SendMessage(hwndCommand, WM_APP+1, 0, 0);
	}
	SendMessage(hwndCommand, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)commandStr);
	return true;
}

bool SecondProcessProxy::SendCaretRange(int startPos, int length)
{
	SPDLOG_DEBUG(_T("args startPos:{0} length:{1}"), startPos, length);

	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (hwnd == NULL) {
		return false;
	}

	HWND hwndCommand = GetDlgItem(hwnd, IDC_EDIT_COMMAND2);
	if (hwndCommand == NULL) {
		SPDLOG_ERROR(_T("CmdReceiveEdit does not found."));
		return false;
	}

	SendMessage(hwndCommand, WM_APP+2, startPos, length);
	return true;
}

/**
 *  指定されたパスをコマンドとして登録する
 *  @return true: 成功 false:失敗
 *  @param pathStr  登録対象のファイルパス
 */
bool SecondProcessProxy::RegisterPath(const CString& pathStr)
{
	SPDLOG_DEBUG(_T("args path:{0}"), (LPCTSTR)pathStr);

	CString name = PathFindFileName(pathStr);
	PathRemoveExtension(name.GetBuffer(name.GetLength()));
	name.ReleaseBuffer();

	// 空白を置換
	name.Replace(_T(' '), _T('_'));

	CString commandStr(_T("new "));
	commandStr += _T("\"") + name + _T("\" \"") + pathStr + _T("\"");
	return SendCommandString(commandStr, false);
}

bool SecondProcessProxy::ChangeDirectory(const CString& pathStr)
{
	UNREFERENCED_PARAMETER(pathStr);
	// FIXME: 実装
	return false;
}

bool SecondProcessProxy::Hide()
{
	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (IsWindow(hwnd) == FALSE) {
		return false;
	}

	PostMessage(hwnd, WM_APP+7, 0, 0);
	return true;
}

/**
 * @return true: アクティブ化した  false: 先行プロセスはない
 */
bool SecondProcessProxy::Show()
{
	SPDLOG_DEBUG(_T("start"));

	// 先行プロセスを有効化する
	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (hwnd == NULL) {
		return false;
	}
	LauncherMainWindow::ActivateWindow(hwnd);
	return true;
}



}


