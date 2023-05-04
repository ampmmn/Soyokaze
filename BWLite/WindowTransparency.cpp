#include "pch.h"
#include "framework.h"
#include "WindowTransparency.h"
#include "AppProfile.h"

WindowTransparency::WindowTransparency() : 
	mWindowHandle(nullptr),
	mAlpha(255),
	mIsEnable(true),
	mIsInactiveOnly(true)
{
	CAppProfile* app = CAppProfile::Get();
	mIsEnable = (app->Get(_T("WindowTransparency"), _T("Enable"), 1) != 0);

	mAlpha = app->Get(_T("WindowTransparency"), _T("Alpha"), 200);
	mIsInactiveOnly =(app->Get(_T("WindowTransparency"), _T("InactiveOnly"), 1) != 0);
}

WindowTransparency::~WindowTransparency()
{
}

void WindowTransparency::SetWindowHandle(HWND hwnd)
{
	mWindowHandle = hwnd;

	if (mIsEnable) {
		// WS_EX_LAYEREDを付与する
		// (ダイアログエディタ上でこのスタイルを設定するとエディタ上のダイアログが黒塗りになるので)
		long exStyle = GetWindowLong(mWindowHandle, GWL_EXSTYLE);
		SetWindowLong(mWindowHandle, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);

		if (mIsInactiveOnly == false) {
			SetLayeredWindowAttributes(mWindowHandle, 0, mAlpha, LWA_ALPHA);
		}

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

