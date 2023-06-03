#pragma once

#include <afx.h>

// 一時的にWS_EX_TOPMOSTを無効化する
class TopMostMask
{
public:
	TopMostMask() : mIsMasked(false), mWindow(nullptr)
	{
		CWnd* wnd = AfxGetMainWnd();
		if (wnd == nullptr) {
			return ;
		}
		mWindow = wnd->GetSafeHwnd();
		long exStyle = ::GetWindowLong(mWindow, GWL_EXSTYLE);
		if ((exStyle & WS_EX_TOPMOST) == 0) {
			return ;
		}

		SetWindowPos(mWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		mIsMasked = true;
	}
	~TopMostMask() {

		if (mIsMasked == false || mWindow == nullptr) {
			return ;
		}
		SetWindowPos(mWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

protected:
	bool mIsMasked;
	HWND mWindow;

};

