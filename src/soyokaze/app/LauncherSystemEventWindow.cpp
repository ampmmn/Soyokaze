#include "pch.h"
#include "LauncherSystemEventWindow.h"
#include "app/LauncherEventDispatcher.h"
#include "core/LauncherEventListenerIF.h"
#include <wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

constexpr UINT_PTR TIMERID_OPERATION = 2;

}

BEGIN_MESSAGE_MAP(LauncherSystemEventWindow, CWnd)
	ON_WM_TIMER()
	ON_MESSAGE(WM_WTSSESSION_CHANGE, OnMessageSessionChange)
END_MESSAGE_MAP()

LauncherSystemEventWindow::LauncherSystemEventWindow()
{
}

LauncherSystemEventWindow::~LauncherSystemEventWindow()
{
	if (IsWindow(GetSafeHwnd())) {
		KillTimer(TIMERID_OPERATION);
		WTSUnRegisterSessionNotification(GetSafeHwnd());
		DestroyWindow();
	}
}

BOOL LauncherSystemEventWindow::Create()
{
	BOOL isOK = CreateEx(WS_EX_TOOLWINDOW, AfxRegisterWndClass(0),
	                     _T("LauncherSystemEventWindow"), WS_OVERLAPPED,
	                     0, 0, 0, 0, nullptr, nullptr);
	if (isOK == FALSE) {
		return FALSE;
	}

	if (WTSRegisterSessionNotification(GetSafeHwnd(), NOTIFY_FOR_ALL_SESSIONS) == FALSE) {
		spdlog::error("Failed to register session notification.");
		DestroyWindow();
		return FALSE;
	}

	if (SetTimer(TIMERID_OPERATION, 1000, nullptr) == 0) {
		spdlog::error("Failed to create operation timer.");
		WTSUnRegisterSessionNotification(GetSafeHwnd());
		DestroyWindow();
		return FALSE;
	}

	return TRUE;
}

void LauncherSystemEventWindow::OnTimer(UINT_PTR timerId)
{
	if (timerId == TIMERID_OPERATION) {
		LauncherEventDispatcher::Get()->Dispatch([](LauncherEventListenerIF* listener) {
			listener->OnTimer();
		});
	}
}

LRESULT LauncherSystemEventWindow::OnMessageSessionChange(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	if (wParam == WTS_SESSION_LOCK) {
		SPDLOG_INFO(_T("WTS_SESSION_LOCK"));
		LauncherEventDispatcher::Get()->Dispatch([](LauncherEventListenerIF* listener) {
			listener->OnLockScreenOccurred();
		});
	}
	else if (wParam == WTS_SESSION_UNLOCK) {
		SPDLOG_INFO(_T("WTS_SESSION_UNLOCK"));
		LauncherEventDispatcher::Get()->Dispatch([](LauncherEventListenerIF* listener) {
			listener->OnUnlockScreenOccurred();
		});
	}
	return 0;
}
