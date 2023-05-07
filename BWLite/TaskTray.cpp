#include "pch.h"
#include "framework.h"
#include "TaskTray.h"
#include "resource.h"
#include "BWLiteDlg.h"
#include "IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int ID_BWLITE_TASKTRAY = 1000;

IMPLEMENT_DYNAMIC(TaskTray, CWnd)

TaskTray::TaskTray(CBWLiteDlg* window) : mBWLiteWindowPtr(window)
{
	mIcon = IconLoader::Get()->LoadTasktrayIcon();
}

TaskTray::~TaskTray()
{
	NOTIFYICONDATA nid;
	nid.cbSize           = sizeof(NOTIFYICONDATA);
	nid.hWnd             = mTaskTrayWindow;
	nid.uID              = ID_BWLITE_TASKTRAY;
	Shell_NotifyIcon(NIM_DELETE, &nid);
}


BEGIN_MESSAGE_MAP(TaskTray, CWnd)
	ON_MESSAGE(WM_APP+1, OnNotifyTrakTray)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL TaskTray::Create()
{
	BOOL isOK = CreateEx(WS_EX_TOOLWINDOW, AfxRegisterWndClass(0),
	                     _T("BWLiteTaskTray"), WS_OVERLAPPED,
	                     0, 0, 0, 0, NULL, NULL );

	if (isOK == FALSE) {
		return FALSE;
	}


	mTaskTrayWindow = GetSafeHwnd();

	// タスクトレイ登録

	NOTIFYICONDATA nid;
	nid.cbSize           = sizeof(NOTIFYICONDATA);
	nid.hWnd             = GetSafeHwnd();
	nid.uID              = ID_BWLITE_TASKTRAY;
	nid.uCallbackMessage = WM_APP+1;
	nid.uFlags           = NIF_MESSAGE;

	nid.uFlags |= NIF_ICON;
	nid.hIcon   = mIcon;

	CString tipsStr;
	tipsStr = _T("BWLite");

	nid.uFlags |= NIF_TIP;
	_tcsncpy_s(nid.szTip, sizeof(nid.szTip), tipsStr, _TRUNCATE);

	return Shell_NotifyIcon(NIM_ADD, &nid);

}

LRESULT TaskTray::OnNotifyTrakTray(WPARAM wp, LPARAM lp)
{
	UINT nID   = (UINT)wp;
	UINT msg = (UINT)lp;
	
	if (nID != ID_BWLITE_TASKTRAY) {
		return 0;
	}

	if (msg == WM_LBUTTONDBLCLK) {
		mBWLiteWindowPtr->ActivateWindow();
	}
	else if (msg == WM_LBUTTONUP || msg == WM_RBUTTONDOWN) {
		OnContextMenu();
	}

	return 0;
}

void TaskTray::OnContextMenu()
{
	mBWLiteWindowPtr->OnContextMenu(this);
}

