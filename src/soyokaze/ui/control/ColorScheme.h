#pragma once

class ColorSchemeIF
{
public:
	virtual ~ColorSchemeIF() {}

// ウインドウ
	// ウインドウの背景ブラシ
	virtual HBRUSH GetBackgroundBrush() = 0;
	// テキストの色
	virtual COLORREF GetTextColor() = 0;
	// 背景色
	virtual COLORREF GetBackgroundColor() = 0;

// 入力欄
	// テキストの色
	virtual COLORREF GetEditTextColor() = 0;
	// 背景色の色
	virtual COLORREF GetEditBackgroundColor() = 0;
	// 背景色のブラシ
	virtual HBRUSH GetEditBackgroundBrush() = 0;
	// キャレット
	virtual COLORREF GetCaretColor(bool isIMEOn) = 0;

// 候補欄
	// テキストの色
	virtual COLORREF GetListTextColor() = 0;
	// 選択項目のテキストの色
	virtual COLORREF GetListHighlightTextColor() = 0;
	// 背景色
	virtual HBRUSH GetListBackgroundBrush() = 0;
	virtual COLORREF GetListBackgroundColor() = 0;
	// 背景色(交互)
	virtual HBRUSH GetListBackgroundAltBrush() = 0;
	virtual COLORREF GetListBackgroundAltColor() = 0;
	// 選択項目の背景の色
	virtual HBRUSH GetListHighlightBackgroundBrush() = 0;
	virtual COLORREF GetListHighlightBackgroundColor() = 0;

};

