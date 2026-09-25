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

MouseoverActivateState::Action MouseoverActivateState::Update(bool isInside, bool isLeftButtonDown)
{
	if (mHasEntered == false) {
		if (isInside) {
			// 初回の侵入は状態を記録するだけにする。
			mHasEntered = true;
			mWasInside = true;
		}
		mWasLeftButtonDown = isLeftButtonDown;
		return Action::None;
	}

	if (isLeftButtonDown) {
		// ドラッグ中は切替を抑止し、ドラッグ開始時の位置を保持する。
		mWasLeftButtonDown = true;
		return Action::None;
	}

	if (mWasLeftButtonDown) {
		// ボタンを離した直後は、ドラッグ完了後の処理を先に行えるよう1回待つ。
		mWasLeftButtonDown = false;
		return Action::None;
	}

	Action action = Action::None;
	if (isInside && mWasInside == false) {
		action = Action::Activate;
	}
	else if (isInside == false && mWasInside) {
		action = Action::Deactivate;
	}

	mWasInside = isInside;
	return action;
}

void MouseoverActivateState::Reset()
{
	mHasEntered = false;
	mWasInside = false;
	mWasLeftButtonDown = false;
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
		if (mIsSuspended == false && AppPreference::Get()->IsMouseoverActivate()) {
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

		bool isLeftButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		auto action = mMouseoverActivateState.Update(isInside, isLeftButtonDown);
		if (action == MouseoverActivateState::Action::Activate) {
			::SetForegroundWindow(mParentHandle);
		}
		else if (action == MouseoverActivateState::Action::Deactivate) {
			if (::GetForegroundWindow() == mParentHandle) {
				HWND nextHwnd = GetNextHwnd();
				if (IsWindow(nextHwnd)) {
					::SetForegroundWindow(nextHwnd);
				}
			}
		}
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

	void Suspend()
	{
		mIsSuspended = true;
		StopTimer();
	}

	void Resume()
	{
		mIsSuspended = false;
		UpdateTimer();
	}

	void ResetState()
	{
		mMouseoverActivateState.Reset();
	}

	HWND mWindowHandle{nullptr};
	HWND mParentHandle{nullptr};
	UINT_PTR mTimerId{0};
	MouseoverActivateState mMouseoverActivateState;
	bool mIsSuspended{false};
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

void MouseoverActivateWindow::Suspend()
{
	in->Suspend();
}

void MouseoverActivateWindow::Resume()
{
	in->Resume();
}

void MouseoverActivateWindow::ResetState()
{
	in->ResetState();
}

void MouseoverActivateWindow::OnTimer(UINT_PTR timerId)
{
	if (timerId == TIMERID_MOUSEOVER) {
		in->OnTimer();
	}
}
