#include "pch.h"
#include "MouseoverActivateWindow.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

constexpr UINT_PTR TIMERID_MOUSEOVER = 1;
constexpr UINT TIMER_INTERVAL = 100;

HWND GetNextHwnd()
{
	HWND hwnd = GetForegroundWindow();
	while (hwnd) {
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);

		if (IsWindow(hwnd) == FALSE) {
			break;
		}
		if (IsWindowVisible(hwnd) == FALSE) {
			continue;
		}
		break;
	}
	return hwnd;
}

}

BEGIN_MESSAGE_MAP(MouseoverActivateWindow, CWnd)
	ON_WM_TIMER()
END_MESSAGE_MAP()

struct MouseoverActivateWindow::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this, _T("MouseoverActivateWindow"));
	}

	~PImpl() override
	{
		StopTimer();
		AppPreference::Get()->UnregisterListener(this);
	}

	void SetWindowHandle(HWND hwnd)
	{
		mWindowHandle = hwnd;
	}

	void SetParentHandle(HWND hwnd)
	{
		mParentHandle = hwnd;
	}

	void UpdateTimer()
	{
		if (AppPreference::Get()->IsMouseoverActivate()) {
			StartTimer();
		}
		else {
			StopTimer();
		}
	}

	void OnTimer()
	{
		if (IsWindow(mParentHandle) == FALSE || ::IsWindowVisible(mParentHandle) == FALSE) {
			ResetState();
			return;
		}

		POINT point;
		if (GetCursorPos(&point) == FALSE) {
			return;
		}

		CRect clientRect;
		::GetClientRect(mParentHandle, &clientRect);
		::ScreenToClient(mParentHandle, &point);
		bool isInside = clientRect.PtInRect(point) != FALSE;

		if (mHasEntered == false) {
			if (isInside) {
				// 初回の侵入は状態を記録するだけにする。
				mHasEntered = true;
				mWasInside = true;
			}
			return;
		}

		if (isInside && mWasInside == false) {
			::SetForegroundWindow(mParentHandle);
		}
		else if (isInside == false && mWasInside) {
			if (::GetForegroundWindow() == mParentHandle) {
				HWND nextHwnd = GetNextHwnd();
				if (IsWindow(nextHwnd)) {
					::SetForegroundWindow(nextHwnd);
				}
			}
		}

		mWasInside = isInside;
	}

	void OnAppPreferenceUpdated() override
	{
		UpdateTimer();
	}

	void OnAppFirstBoot() override
	{
	}

	void OnAppNormalBoot() override
	{
	}

	void OnAppExit() override
	{
	}

	void StartTimer()
	{
		if (mTimerId != 0 || IsWindow(mWindowHandle) == FALSE) {
			return;
		}

		mTimerId = ::SetTimer(mWindowHandle, TIMERID_MOUSEOVER, TIMER_INTERVAL, nullptr);
	}

	void StopTimer()
	{
		if (mTimerId != 0 && IsWindow(mWindowHandle)) {
			::KillTimer(mWindowHandle, mTimerId);
		}
		mTimerId = 0;
		ResetState();
	}

	void ResetState()
	{
		mHasEntered = false;
		mWasInside = false;
	}

	HWND mWindowHandle{nullptr};
	HWND mParentHandle{nullptr};
	UINT_PTR mTimerId{0};
	bool mHasEntered{false};
	bool mWasInside{false};
};

MouseoverActivateWindow::MouseoverActivateWindow() : in(std::make_unique<PImpl>())
{
}

MouseoverActivateWindow::~MouseoverActivateWindow()
{
	if (IsWindow(GetSafeHwnd())) {
		DestroyWindow();
	}
}

BOOL MouseoverActivateWindow::Create(CWnd* parentWnd)
{
	if (parentWnd == nullptr || IsWindow(parentWnd->GetSafeHwnd()) == FALSE) {
		return FALSE;
	}

	CRect rect(0, 0, 0, 0);
	BOOL result = CWnd::CreateEx(
		0,
		AfxRegisterWndClass(0),
		_T("MouseoverActivateWindow"),
		WS_CHILD,
		rect,
		parentWnd,
		0);
	if (result == FALSE) {
		return FALSE;
	}

	in->SetWindowHandle(GetSafeHwnd());
	in->SetParentHandle(parentWnd->GetSafeHwnd());
	in->UpdateTimer();
	ShowWindow(SW_HIDE);
	return TRUE;
}

void MouseoverActivateWindow::OnTimer(UINT_PTR timerId)
{
	if (timerId == TIMERID_MOUSEOVER) {
		in->OnTimer();
	}
}
