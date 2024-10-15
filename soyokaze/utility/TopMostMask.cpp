#include "pch.h"
#include "TopMostMask.h"
#include "SharedHwnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TopMostMask::TopMostMask() : mIsMasked(false), mWindow(nullptr)
{
	SharedHwnd hwnd;
	mWindow = hwnd.GetHwnd();
	if (mWindow == nullptr) {
		return ;
	}
	long exStyle = ::GetWindowLong(mWindow, GWL_EXSTYLE);
	if ((exStyle & WS_EX_TOPMOST) == 0) {
		return ;
	}

	SetWindowPos(mWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	mIsMasked = true;
}

TopMostMask::~TopMostMask()
{
	if (mIsMasked == false || mWindow == nullptr) {
		return ;
	}
	SetWindowPos(mWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

