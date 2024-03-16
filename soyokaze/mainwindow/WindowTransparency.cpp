#include "pch.h"
#include "framework.h"
#include "WindowTransparency.h"
#include "setting/AppPreference.h"

WindowTransparency::WindowTransparency() : 
	mWindowHandle(nullptr),
	mAlpha(255),
	mIsEnable(true),
	mIsInactiveOnly(true),
	mIsTopmost(true),
	mIsHideOnInactive(false)
{
	AppPreference::Get()->RegisterListener(this);
}

WindowTransparency::~WindowTransparency()
{
	AppPreference::Get()->UnregisterListener(this);
}

void WindowTransparency::SetWindowHandle(HWND hwnd)
{
	mWindowHandle = hwnd;

	UpdateStyle();

}

bool WindowTransparency::UpdateActiveState(UINT nState)
{
	// ウインドウが無効なら何もしない
	if (IsWindow(mWindowHandle) == FALSE) {
		return false;
	}

	// フォーカスをうしなったらウインドウを隠す
	if (mIsHideOnInactive && nState == WA_INACTIVE) {
		ShowWindow(mWindowHandle, SW_HIDE);
	}

	// 透過表示機能が無効なら何もしない
	if (mIsEnable == false) {
		return false;
	}

	// 透過度を設定する
	BYTE alpha = mAlpha;

	if (mIsInactiveOnly && nState != WA_INACTIVE) {
		alpha = 0xff;
	}

	SetLayeredWindowAttributes(mWindowHandle, 0, alpha, LWA_ALPHA);

	return true;
}

bool WindowTransparency::ToggleAlphaState(bool isTransparency)
{
	// ウインドウが無効なら何もしない
	if (IsWindow(mWindowHandle) == FALSE) {
		return false;
	}

	// 透過表示機能が無効なら何もしない
	if (mIsEnable == false) {
		return false;
	}

	// 透過度を設定する
	BYTE alpha = mAlpha;

	if (mIsInactiveOnly && isTransparency == false) {
		alpha = 0xff;
	}

	SetLayeredWindowAttributes(mWindowHandle, 0, alpha, LWA_ALPHA);

	return true;
}

void WindowTransparency::UpdateStyle()
{
	auto* pref = AppPreference::Get();
	mIsEnable = pref->IsWindowTransparencyEnable();
	mAlpha = pref->GetAlpha();
	mIsInactiveOnly = pref->IsTransparencyInactiveOnly();
	mIsTopmost = pref->IsTopMost();
	mIsHideOnInactive = pref->IsHideOnInactive();

	// 最上位に表示する場合
	if (mIsTopmost) {
		SetWindowPos(mWindowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else {
		SetWindowPos(mWindowHandle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	long exStyle = GetWindowLong(mWindowHandle, GWL_EXSTYLE);
	if (mIsEnable) {
		// WS_EX_LAYEREDを付与する
		// (ダイアログエディタ上でこのスタイルを設定するとエディタ上のダイアログが黒塗りになるので)
		exStyle |= WS_EX_LAYERED;
	}
	else {
		exStyle &= (~WS_EX_LAYERED);
	}

	SetWindowLong(mWindowHandle, GWL_EXSTYLE, exStyle);

	if (mIsEnable && mIsInactiveOnly == false) {
		// 常に透明にする設定の場合は、この時点で透過率を設定しておく
		SetLayeredWindowAttributes(mWindowHandle, 0, mAlpha, LWA_ALPHA);
	}
}

void WindowTransparency::OnAppFirstBoot()
{
}

void WindowTransparency::OnAppPreferenceUpdated()
{
	UpdateStyle();
}

void WindowTransparency::OnAppExit()
{
	AppPreference::Get()->UnregisterListener(this);
}

