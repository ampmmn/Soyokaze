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
constexpr UINT_PTR TIMERID_MONITOR_CONFIGURATION = 3;
constexpr UINT MONITOR_CONFIGURATION_DEBOUNCE_MSEC = 500;

}

BEGIN_MESSAGE_MAP(LauncherSystemEventWindow, CWnd)
	ON_WM_TIMER()
	ON_MESSAGE(WM_WTSSESSION_CHANGE, OnMessageSessionChange)
	ON_MESSAGE(WM_DEVICECHANGE, OnMessageMonitorConfigurationChange)
	ON_MESSAGE(WM_DISPLAYCHANGE, OnMessageMonitorConfigurationChange)
	ON_MESSAGE(WM_SETTINGCHANGE, OnMessageMonitorConfigurationChange)
END_MESSAGE_MAP()

LauncherSystemEventWindow::LauncherSystemEventWindow()
{
}

LauncherSystemEventWindow::~LauncherSystemEventWindow()
{
	if (IsWindow(GetSafeHwnd())) {
		KillTimer(TIMERID_OPERATION);
		KillTimer(TIMERID_MONITOR_CONFIGURATION);
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
	else if (timerId == TIMERID_MONITOR_CONFIGURATION) {
		KillTimer(TIMERID_MONITOR_CONFIGURATION);
		LauncherEventDispatcher::Get()->Dispatch([](LauncherEventListenerIF* listener) {
			listener->OnMonitorConfigurationChanged();
		});
	}
}

LRESULT LauncherSystemEventWindow::OnMessageMonitorConfigurationChange(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	KillTimer(TIMERID_MONITOR_CONFIGURATION);
	if (SetTimer(TIMERID_MONITOR_CONFIGURATION, MONITOR_CONFIGURATION_DEBOUNCE_MSEC, nullptr) == 0) {
		spdlog::error("Failed to set monitor configuration debounce timer.");
		LauncherEventDispatcher::Get()->Dispatch([](LauncherEventListenerIF* listener) {
			listener->OnMonitorConfigurationChanged();
		});
	}
	return 0;
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
