#pragma once

#include <afx.h>

// 一時的にWS_EX_TOPMOSTを無効化する
class TopMostMask
{
public:
	TopMostMask() : mIsMasked(false)
	{
		CWnd* wnd = AfxGetMainWnd();
		if (wnd == nullptr) {
			return ;
		}
		long exStyle = ::GetWindowLong(wnd->GetSafeHwnd(), GWL_EXSTYLE);
		if ((exStyle & WS_EX_TOPMOST) == 0) {
			return ;
		}

		SetWindowPos(wnd->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		mIsMasked = true;
	}
	~TopMostMask() {

		if (mIsMasked == false) {
			return ;
		}

		CWnd* wnd = AfxGetMainWnd();
		if (wnd == nullptr) {
			return ;
		}
		SetWindowPos(wnd->GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

protected:
	bool mIsMasked;

};

