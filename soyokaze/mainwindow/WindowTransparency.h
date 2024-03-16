#pragma once

#include "setting/AppPreferenceListenerIF.h"
#include <stdint.h>

class WindowTransparency : public AppPreferenceListenerIF
{
public:
	WindowTransparency();
	virtual ~WindowTransparency();

	void SetWindowHandle(HWND hwnd);

	// ウインドウのアクティブ状態を更新して状態に応じた制御を行う
	bool UpdateActiveState(UINT nState);

	// 透過状態を(一時的に)変更する
	bool ToggleAlphaState(bool isTransparency);

protected:
	void UpdateStyle();


	void OnAppFirstBoot() override;
	void OnAppPreferenceUpdated() override;
	void OnAppExit() override;

protected:
	// 対象ウインドウハンドル
	HWND mWindowHandle;

	// 透過度(0:透明 255:不透明)
	uint8_t mAlpha;

	// 透過表示機能を利用するか?
	bool mIsEnable;

	// 非アクティブのときだけ透明にする
	bool mIsInactiveOnly;

	// 非アクティブになったらウインドウを隠す
	bool mIsHideOnInactive;

	// 最上位で表示する
	bool mIsTopmost;
	   // ToDo: 別の位置に移動するかクラス名を変更する
};

