#include "pch.h"
#include "framework.h"
#include "WindowTransparency.h"
#include "AppPreference.h"

WindowTransparency::WindowTransparency() : 
	mWindowHandle(nullptr),
	mAlpha(255),
	mIsEnable(true),
	mIsInactiveOnly(true),
	mIsTopmost(true)
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
	if (IsWindow(mWindowHandle) == FALSE) {
		return false;
	}
	if (mIsEnable == false) {
		return false;
	}

	BYTE alpha = mAlpha;

	if (mIsInactiveOnly && nState != WA_INACTIVE) {
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

