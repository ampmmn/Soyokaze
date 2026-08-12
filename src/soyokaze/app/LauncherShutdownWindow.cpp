#include "pch.h"
#include "LauncherShutdownWindow.h"
#include "core/LauncherProcessContext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(LauncherShutdownWindow, CWnd)
	ON_WM_QUERYENDSESSION()
END_MESSAGE_MAP()

BOOL LauncherShutdownWindow::Create()
{
	return CreateEx(WS_EX_TOOLWINDOW, AfxRegisterWndClass(0),
	               _T("LauncherShutdownWindow"), WS_OVERLAPPED,
	               0, 0, 0, 0, nullptr, nullptr);
}

BOOL LauncherShutdownWindow::OnQueryEndSession()
{
	SPDLOG_INFO(_T("Launcher app shutdown is in progress."));

	launcherapp::core::LauncherProcessContext::GetInstance()->MarkShutdownInProgress();

	DestroyWindow();
	PostQuitMessage(0);

	return TRUE;
}
