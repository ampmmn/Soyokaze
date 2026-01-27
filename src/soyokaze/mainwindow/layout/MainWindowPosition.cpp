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
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	if (GetWindowPlacement(hwnd, &wp) == false) {
		return false;
	}

	int orgHeight = mPosition.rcNormalPosition.bottom - mPosition.rcNormalPosition.top; 

	wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + orgHeight;

	mPosition = wp;
	return true;
}

bool MainWindowPosition::SetPositionTemporary(HWND hwnd, const CRect& rc)
{
	spdlog::debug("MainWindowPosition::SetPositionTemporary start");

	auto wp = mPosition;
	wp.rcNormalPosition = rc;

	if (IsZoomed(hwnd) == FALSE && IsIconic(hwnd) == FALSE) {
		SetWindowPos(hwnd, nullptr,rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
	}
	return true;
}

bool MainWindowPosition::SyncPosition(HWND hwnd)
{
	spdlog::debug("MainWindowPosition::SyncPosition start");

	if (mIsLoaded == false) {
		GetWindowRect(hwnd, &mPosition.rcNormalPosition);
		mPosition.rcNormalPosition.bottom = mPosition.rcNormalPosition.top + DEFAULT_HEIGHT;

	}
	return SetWindowPlacement(hwnd, &mPosition) != FALSE;  // ToDo これの先にClearContent
}


