#include "pch.h"
#include "RegionIndicatorWindow.h"
#include "SharedHwnd.h"

namespace launcherapp { namespace commands { namespace activate_window {

constexpr UINT TIMERID_WATCHWINDOW = 1;

RegionIndicatorWindow::RegionIndicatorWindow()
{
}

RegionIndicatorWindow::~RegionIndicatorWindow()
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

RegionIndicatorWindow* RegionIndicatorWindow::GetInstance()
{
	static RegionIndicatorWindow inst;
	return &inst;
}

void RegionIndicatorWindow::SetIndependentMode(bool isIndependent)
{
	// WM_TIMERでメインウインドウが前面にあるかどうかの監視をするかしないかを制御
	mIsIndependent = isIndependent;
}

// 指定したhwndの領域を覆う形でウインドウを表示
void RegionIndicatorWindow::Cover(RECT rc)
{
	if (mHwnd == nullptr) {
		mHwnd = Create();
	}

	// ウインドウ領域を記憶しておく
	mRectWindow = rc;

	Draw();
}

void RegionIndicatorWindow::Draw()
{
	if (mBuffer) {
		DeleteObject(mBuffer);
		mBuffer = nullptr;
	}

	const auto& rc = mRectWindow;

	constexpr int BORDER = 14;
	constexpr int HALF_BORDER = BORDER / 2;

	// 枠の描画
	HDC dc = GetDC(mHwnd);
	HDC memDC = CreateCompatibleDC(dc);
	mBuffer = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
	HPEN pen = CreatePen(PS_SOLID, BORDER, RGB(0, 0, 255));
	HBRUSH br = CreateSolidBrush(RGB(0, 0, 255));
	auto orgBr = SelectObject(memDC, br);
	auto orgPen = SelectObject(memDC, pen);
	auto orgBmp = SelectObject(memDC, mBuffer);
	Rectangle(memDC, HALF_BORDER, HALF_BORDER, rc.Width() - HALF_BORDER, rc.Height() - HALF_BORDER);
	SelectObject(memDC, orgBmp);
	SelectObject(memDC, orgPen);
	SelectObject(memDC, orgBr);
	DeleteObject(br);
	DeleteObject(pen);
	DeleteDC(memDC);
	ReleaseDC(mHwnd, dc);

	// リージョンを作成
	HRGN hRgn = CreateRectRgn(0, 0, rc.Width(), rc.Height());
	//HRGN hRgn1 = CreateRectRgn(0, 0, rc.Width(), rc.Height());
	//HRGN hRgn2 = CreateRectRgn(BORDER, BORDER, rc.Width() - BORDER, rc.Height() - BORDER);
	//CombineRgn(hRgn, hRgn1, hRgn2, RGN_DIFF);
	SetWindowRgn(mHwnd, hRgn, FALSE);
	DeleteObject(hRgn);

	// 強調したいウインドウに重ねて表示
	SharedHwnd mainWnd;
	auto hwndPrev = GetWindow(mainWnd.GetHwnd(), GW_HWNDNEXT);
	SetWindowPos(mHwnd, hwndPrev, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	ShowWindow(mHwnd, SW_SHOWNOACTIVATE);
	RedrawWindow(mHwnd, nullptr, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ALLCHILDREN | RDW_ERASE);
}


void RegionIndicatorWindow::Uncover()
{
	ShowWindow(mHwnd, SW_HIDE);
}

LRESULT CALLBACK RegionIndicatorWindow::OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_PAINT) {

		// 描画済みの画面を張り付け
		auto thisPtr = (RegionIndicatorWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		return thisPtr->OnPaint(hwnd, wp, lp);
	}
	else if (msg == WM_TIMER && wp == TIMERID_WATCHWINDOW) {
		// 一定間隔でフォーカスのチェックをする
		auto thisPtr = (RegionIndicatorWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		return thisPtr->OnTimer(hwnd, wp, lp);
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT RegionIndicatorWindow::OnPaint(HWND hwnd, WPARAM wp, LPARAM lp)
{
	CRect rc;
	GetWindowRect(hwnd, &rc);
	if (rc.left != mRectWindow.left || rc.top != mRectWindow.top) {

		// 環境により、Coverメソッド内の末尾で変更したウインドウ位置がこの時点で反映されていないケースがみられるため、
		// その場合はここで位置とサイズ変更を再度行う
		spdlog::info("[RegionIndicatorWindow] window position reset {0},{1} -> {2},{3}",
		             rc.left, rc.top, mRectWindow.left, mRectWindow.top);

		rc = mRectWindow;
		SetWindowPos(hwnd, nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		RedrawWindow(hwnd, nullptr, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ALLCHILDREN | RDW_ERASE);
		return DefWindowProc(hwnd, WM_PAINT, wp, lp);
	}

	// Coverメソッド内で作成したビットマップを描画
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(hwnd, &ps);
	HDC memDC = CreateCompatibleDC(dc);
	auto orgBmp = SelectObject(memDC, mBuffer);
	BitBlt(dc, 0, 0, rc.Width(), rc.Height(), memDC, 0, 0, SRCCOPY);
	SelectObject(memDC, orgBmp);
	DeleteDC(memDC);

	EndPaint(hwnd, &ps);
	return 0;
}

LRESULT RegionIndicatorWindow::OnTimer(HWND hwnd, WPARAM wp, LPARAM lp)
{
	if (mIsIndependent) {
		// 特定のウインドウと連動しない場合は何もしない
		return 0;
	}

	SharedHwnd mainWnd;
	if (mainWnd.GetHwnd() == GetForegroundWindow()) {
		return 0;
	}

	// フォーカスが外れたらウインドウを非表示にする

	spdlog::info("RegionIndicatorWindow hide.");
	Uncover();
	return 0;
}

HWND RegionIndicatorWindow::Create()
{
	spdlog::info("RegionIndicatorWindow create.");

	// 内部のmessage処理用の不可視のウインドウを作っておく
	HINSTANCE hInst = GetModuleHandle(nullptr);
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("LncrRegionIndicator"), 0, 
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
