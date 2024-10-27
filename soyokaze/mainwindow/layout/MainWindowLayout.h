#pragma once

#include "mainwindow/MainWindowLayoutIF.h"

// メインウインドウのリサイズ時にウインドウ上の部品の位置とサイズを再計算し配置する
class MainWindowLayout : public MainWindowLayoutIF
{
public:
	MainWindowLayout();
	~MainWindowLayout() override;

	// リサイズ中のメインウインドウのサイズ制限を決定する
	void RecalcWindowSize(HWND hwnd, UINT side, LPRECT rect) override;
	// リサイズ時のコントロール位置・サイズの再計算
	void RecalcControls(HWND hwnd) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

