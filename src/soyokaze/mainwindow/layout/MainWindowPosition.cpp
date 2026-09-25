#include "pch.h"
#include "MainWindowPosition.h"
#include "utility/AppProfile.h"
#include "app/AppName.h"
#include <utility> // for std::pair

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr int DEFAULT_HEIGHT = 480;

MainWindowPosition::MainWindowPosition() : WindowPosition()
{
}

MainWindowPosition::MainWindowPosition(LPCTSTR name) : WindowPosition(name)
{
}

MainWindowPosition::~MainWindowPosition()
{
}

bool MainWindowPosition::UpdateExceptHeight(HWND hwnd)
{
	if (IsCurrentMonitorConfiguration() == false) {
		return false;
	}

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	if (GetWindowPlacement(hwnd, &wp) == false) {
		return false;
	}

	WINDOWPLACEMENT position = GetPosition();
	int orgHeight = position.rcNormalPosition.bottom - position.rcNormalPosition.top;

	wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + orgHeight;

	SetPosition(wp);
	return true;
}

bool MainWindowPosition::SetPositionTemporary(HWND hwnd, const CRect& rc)
{
	spdlog::debug("MainWindowPosition::SetPositionTemporary start");

	auto wp = GetPosition();
	wp.rcNormalPosition = rc;

	if (IsZoomed(hwnd) == FALSE && IsIconic(hwnd) == FALSE) {
		SetWindowPos(hwnd, nullptr,rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
	}
	return true;
}

bool MainWindowPosition::SyncPosition(HWND hwnd)
{
	spdlog::debug("MainWindowPosition::SyncPosition start");

	if (IsPositionLoaded() == false) {
		WINDOWPLACEMENT position = GetPosition();
		GetWindowRect(hwnd, &position.rcNormalPosition);
		position.rcNormalPosition.bottom = position.rcNormalPosition.top + DEFAULT_HEIGHT;
		SetPosition(position);

	}
	WINDOWPLACEMENT placement = GetPosition();
	if (IsWindowVisible(hwnd) == FALSE) {
		// 保存位置のshowCmdで、非表示中のウインドウを表示しない
		placement.showCmd = SW_HIDE;
	}
	return SetWindowPlacement(hwnd, &placement) != FALSE;  // ToDo これの先にClearContent
}


