#include "pch.h"
#include "ActivateIndicatorWindow.h"
#include "SharedHwnd.h"

namespace launcherapp { namespace commands { namespace activate_window {

constexpr UINT TIMERID_WATCHWINDOW = 1;

ActivateIndicatorWindow::ActivateIndicatorWindow()
{
}

ActivateIndicatorWindow::~ActivateIndicatorWindow()
{
	if (mBuffer) {
		DeleteObject(mBuffer);
		mBuffer = nullptr;
	}
	if (mHwnd) {
		KillTimer(mHwnd, TIMERID_WATCHWINDOW);
		DestroyWindow(mHwnd);
		mHwnd = nullptr;
	}
}

ActivateIndicatorWindow* ActivateIndicatorWindow::GetInstance()
{
	static ActivateIndicatorWindow inst;
	return &inst;
}

// 指定したhwndの領域を覆う形でウインドウを表示
void ActivateIndicatorWindow::Cover(HWND hwnd)
{
	if (mHwnd == nullptr) {
		mHwnd = Create();
	}

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (style & WS_MINIMIZE) {
		// hwndが最小化されている場合は領域を覆う必要がないため、非表示にする
		ShowWindow(mHwnd, SW_HIDE);
		return;
	}

	// 領域を取得する
	CRect rc;
	GetWindowRect(hwnd, &rc);

	// ウインドウ領域を記憶しておく
	mRectWindow = rc;

	if (mBuffer) {
		DeleteObject(mBuffer);
		mBuffer = nullptr;
	}

	constexpr int BORDER = 14;
	constexpr int HALF_BORDER = BORDER / 2;

	// 枠の描画
	HDC dc = GetDC(mHwnd);
	HDC memDC = CreateCompatibleDC(dc);
	mBuffer = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
	HPEN pen = CreatePen(PS_SOLID, BORDER, RGB(255, 0, 0));
	auto orgBr = SelectObject(memDC, GetStockObject(NULL_BRUSH));
	auto orgPen = SelectObject(memDC, pen);
	auto orgBmp = SelectObject(memDC, mBuffer);
	Rectangle(memDC, HALF_BORDER, HALF_BORDER, rc.Width() - HALF_BORDER, rc.Height() - HALF_BORDER);
	SelectObject(memDC, orgBmp);
	SelectObject(memDC, orgPen);
	SelectObject(memDC, orgBr);
	DeleteObject(pen);
	DeleteDC(memDC);
	ReleaseDC(mHwnd, dc);

	// リージョンを作成
	HRGN hRgn = CreateRectRgn(0, 0, rc.Width(), rc.Height());
	HRGN hRgn1 = CreateRectRgn(0, 0, rc.Width(), rc.Height());
	HRGN hRgn2 = CreateRectRgn(BORDER, BORDER, rc.Width() - BORDER, rc.Height() - BORDER);
	CombineRgn(hRgn, hRgn1, hRgn2, RGN_DIFF);
	SetWindowRgn(mHwnd, hRgn, FALSE);
	DeleteObject(hRgn1);
	DeleteObject(hRgn2);
	DeleteObject(hRgn);

	// 強調したいウインドウに重ねて表示
	SharedHwnd mainWnd;
	auto hwndPrev = GetWindow(mainWnd.GetHwnd(), GW_HWNDNEXT);
	SetWindowPos(mHwnd, hwndPrev, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	ShowWindow(mHwnd, SW_SHOWNOACTIVATE);
	RedrawWindow(mHwnd, nullptr, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ALLCHILDREN | RDW_ERASE);
}


void ActivateIndicatorWindow::Uncover()
{
	ShowWindow(mHwnd, SW_HIDE);
}

LRESULT CALLBACK ActivateIndicatorWindow::OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_PAINT) {

		auto thisPtr = (ActivateIndicatorWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		CRect rc;
		GetWindowRect(hwnd, &rc);
		if (rc.left != thisPtr->mRectWindow.left || rc.top != thisPtr->mRectWindow.top) {
			// 環境により、Coverメソッド内の末尾で変更したウインドウ位置がこの時点で反映されていないケースがみられるため、
			// その場合はここで位置とサイズ変更を再度行う
			spdlog::info("[ActivateIndicatorWindow] window position reset {0},{1} -> {2},{3}",
			             rc.left, rc.top, thisPtr->mRectWindow.left, thisPtr->mRectWindow.top);
			rc = thisPtr->mRectWindow;
			SetWindowPos(hwnd, nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
			RedrawWindow(hwnd, nullptr, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ALLCHILDREN | RDW_ERASE);
			return DefWindowProc(hwnd, msg, wp, lp);
		}

		// Coverメソッド内で作成したビットマップを描画
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hwnd, &ps);
		HDC memDC = CreateCompatibleDC(dc);
		auto orgBmp = SelectObject(memDC, thisPtr->mBuffer);
		BitBlt(dc, 0, 0, rc.Width(), rc.Height(), memDC, 0, 0, SRCCOPY);
		SelectObject(memDC, orgBmp);
		DeleteDC(memDC);

		EndPaint(hwnd, &ps);
		return 0;
	}
	else if (msg == WM_TIMER && wp == TIMERID_WATCHWINDOW) {
		// フォーカスが外れたら非表示にする

		auto h = GetForegroundWindow();

		SharedHwnd mainWnd;
		if (mainWnd.GetHwnd() != h) {
			auto thisPtr = (ActivateIndicatorWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			thisPtr->Uncover();
		}
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

HWND ActivateIndicatorWindow::Create()
{
	spdlog::info("ActivateIndicatorWindow create.");

	// 内部のmessage処理用の不可視のウインドウを作っておく
	HINSTANCE hInst = GetModuleHandle(nullptr);
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("LncrActivateIndicator"), 0, 
	                           0, 0, 1, 1,
	                           NULL, NULL, hInst, NULL);
	ASSERT(hwnd);

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);

	LONG_PTR styleEx = WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE;
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, styleEx);
	SetWindowLongPtr(hwnd, GWL_STYLE, style & (~WS_CAPTION));
	SetLayeredWindowAttributes(hwnd, 0, 128, LWA_ALPHA);

	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	// アクティブなウインドウを監視用のタイマーを作っておく
	::SetTimer(hwnd, TIMERID_WATCHWINDOW, 100, 0);

	return hwnd;
}

}}} // end of namespace launcherapp::commands::activate_window
