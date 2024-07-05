// あ
#include "pch.h"
#include "framework.h"
#include "TaskTray.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int ID_LAUNCHER_TASKTRAY = 1000;

IMPLEMENT_DYNAMIC(TaskTray, CWnd)

TaskTray::TaskTray(TaskTrayEventListenerIF* listener) : mListenerPtr(listener)
{
	memset(&mNotifyIconData, 0, sizeof(mNotifyIconData));
}

TaskTray::~TaskTray()
{
	Shell_NotifyIcon(NIM_DELETE, &mNotifyIconData);
}


BEGIN_MESSAGE_MAP(TaskTray, CWnd)
	ON_MESSAGE(WM_APP+1, OnNotifyTrakTray)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL TaskTray::Create()
{
	BOOL isOK = CreateEx(WS_EX_TOOLWINDOW, AfxRegisterWndClass(0),
	                     _T("LauncherTaskTray"), WS_OVERLAPPED,
	                     0, 0, 0, 0, NULL, NULL );

	if (isOK == FALSE) {
		return FALSE;
	}


	mTaskTrayWindow = GetSafeHwnd();

	// タスクトレイ登録

	NOTIFYICONDATA nid;
	nid.cbSize           = sizeof(NOTIFYICONDATA);
	nid.hWnd             = GetSafeHwnd();
	nid.uID              = ID_LAUNCHER_TASKTRAY;
	nid.uCallbackMessage = WM_APP+1;
	nid.uFlags           = NIF_MESSAGE;

	nid.uFlags |= NIF_ICON;
	nid.hIcon   = IconLoader::Get()->LoadTasktrayIcon();

	TCHAR moduleName[MAX_PATH_NTFS];
	GetModuleFileName(NULL, moduleName, MAX_PATH_NTFS);
	PathRemoveExtension(moduleName);

	CString tipsStr;
	tipsStr = PathFindFileName(moduleName);

	nid.uFlags |= NIF_TIP;
	_tcsncpy_s(nid.szTip, sizeof(nid.szTip), tipsStr, _TRUNCATE);

	mNotifyIconData = nid;

	return Shell_NotifyIcon(NIM_ADD, &mNotifyIconData);

}

void TaskTray::ShowMessage(const CString& msg)
{
	NOTIFYICONDATA nid = mNotifyIconData;

	nid.cbSize = NOTIFYICONDATA_V3_SIZE;
	nid.uFlags |= NIF_INFO;
	_tcsncpy_s(nid.szInfoTitle, _T(""), _TRUNCATE);
	_tcsncpy_s(nid.szInfo, msg, _TRUNCATE);
	nid.uFlags |= NIF_ICON;
	nid.hIcon  = IconLoader::Get()->LoadTasktrayIcon();
	nid.dwInfoFlags = NIIF_INFO;
	// Note: 任意のアイコンを表示する場合は下記
	// nid.dwInfoFlags = NIIF_USER;
	// nid.hBalloonIcon = IconLoader::Get()->LoadTasktrayIcon();   // ここで任意のアイコン

	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void TaskTray::ShowMessage(const CString& msg, const CString& title)
{
	NOTIFYICONDATA nid = mNotifyIconData;
	nid.cbSize = NOTIFYICONDATA_V3_SIZE;
	nid.uFlags |= NIF_INFO;
	_tcsncpy_s(nid.szInfoTitle, title, _TRUNCATE);
	_tcsncpy_s(nid.szInfo, msg, _TRUNCATE);
	nid.uFlags |= NIF_ICON;
	nid.hIcon  = IconLoader::Get()->LoadTasktrayIcon();
	nid.dwInfoFlags = NIIF_INFO;

	Shell_NotifyIcon(NIM_MODIFY, &nid);
}


LRESULT TaskTray::OnNotifyTrakTray(WPARAM wp, LPARAM lp)
{
	UINT nID   = (UINT)wp;
	UINT msg = (UINT)LOWORD(lp);
	
	if (nID != ID_LAUNCHER_TASKTRAY) {
		return 0;
	}

	if (msg == WM_LBUTTONDBLCLK) {
		mListenerPtr->OnTaskTrayLButtonDblclk();
	}
	else if (msg == WM_RBUTTONDOWN) {
		OnContextMenu();
	}

	return 0;
}

void TaskTray::OnContextMenu()
{
	CPoint pos;
	GetCursorPos(&pos);
	mListenerPtr->OnTaskTrayContextMenu(this, pos);
}

