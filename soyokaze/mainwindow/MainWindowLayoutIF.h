#pragma once

// メインウインドウのコントロールの配置(リサイズ)処理を実装するためのインタフェース
class MainWindowLayoutIF
{
public:
	virtual ~MainWindowLayoutIF() {}

	virtual void RecalcWindowSize(HWND hwnd, UINT side, LPRECT rect) = 0;
	virtual void RecalcControls(HWND hwnd) = 0;
};

