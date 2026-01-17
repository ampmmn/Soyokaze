#pragma once

#include "control/ColorScheme.h"
#include <memory>

class CustomColorScheme : public ColorSchemeIF
{
public:
	CustomColorScheme();
	~CustomColorScheme() override;

	void Reload();

// ウインドウ
	// ウインドウの背景ブラシ
	HBRUSH GetBackgroundBrush() override;
	// テキストの色
	COLORREF GetTextColor() override;
	// 背景色
	COLORREF GetBackgroundColor() override;

// 入力欄
	// テキストの色
	COLORREF GetEditTextColor() override;
	// 背景色の色
	COLORREF GetEditBackgroundColor() override;
	// 背景色のブラシ
	HBRUSH GetEditBackgroundBrush() override;
	// キャレット
	COLORREF GetCaretColor(bool isIMEOn) override;

// 候補欄
	// テキストの色
	COLORREF GetListTextColor() override;
	// 選択項目のテキストの色
	COLORREF GetListHighlightTextColor() override;
	// 背景色
	HBRUSH GetListBackgroundBrush() override;
	COLORREF GetListBackgroundColor() override;
	// 背景色(交互)
	HBRUSH GetListBackgroundAltBrush() override;
	COLORREF GetListBackgroundAltColor() override;
	// 選択項目の背景の色
	HBRUSH GetListHighlightBackgroundBrush() override;
	COLORREF GetListHighlightBackgroundColor() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

