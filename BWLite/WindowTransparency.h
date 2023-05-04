#pragma once

#include <stdint.h>

class WindowTransparency
{
public:
	WindowTransparency();
	~WindowTransparency();

	void SetWindowHandle(HWND hwnd);

	bool UpdateActiveState(UINT nState);
protected:
	// 対象ウインドウハンドル
	HWND mWindowHandle;

	// 透過度(0:透明 255:不透明)
	uint8_t mAlpha;

	// 透過表示機能を利用するか?
	bool mIsEnable;

	// 非アクティブのときだけ透明にする
	bool mIsInactiveOnly;

};

