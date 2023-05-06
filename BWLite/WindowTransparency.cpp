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
}

WindowTransparency::~WindowTransparency()
{
}

void WindowTransparency::SetWindowHandle(HWND hwnd)
{
	mWindowHandle = hwnd;

	auto* pref = AppPreference::Get();
	mIsEnable = pref->mIsTransparencyEnable;
	mAlpha = pref->mAlpha;
	mIsInactiveOnly = pref->mIsTransparencyInactiveOnly;
	mIsTopmost = pref->mIsTopmost;

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

