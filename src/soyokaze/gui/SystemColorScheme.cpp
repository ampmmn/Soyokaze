#include "pch.h"
#include "SystemColorScheme.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct SystemColorScheme::PImpl
{
	CBrush mWindowBkBrush;
	COLORREF mWindowBkColor{0};
	
	CBrush mEditBkBrush;
	COLORREF mEditBkColor{0};
	
	CBrush mListBackgroundBrush;
	COLORREF mListBackgroundColor{0};
	
	CBrush mListBackgroundAltBrush;
	COLORREF mListBackgroundAltColor{0};
	
	CBrush mListHighlightBackgroundBrush;
	COLORREF mListHighlightBackgroundColor{0};
};

SystemColorScheme::SystemColorScheme() : in(new PImpl)
{
}

SystemColorScheme::~SystemColorScheme() 
{
}

// ウインドウの背景ブラシ
HBRUSH SystemColorScheme::GetBackgroundBrush() 
{
	COLORREF cr = GetBackgroundColor(); 

	if (in->mWindowBkBrush != (HBRUSH)nullptr && cr == in->mWindowBkColor) {
		return in->mWindowBkBrush;
	}

	in->mWindowBkColor = cr;
	in->mWindowBkBrush.DeleteObject();
	in->mWindowBkBrush.CreateSolidBrush(cr);
	return in->mWindowBkBrush;
}

// テキストの色
COLORREF SystemColorScheme::GetTextColor() 
{
	return GetSysColor(COLOR_WINDOWTEXT);
}

// 背景色
COLORREF SystemColorScheme::GetBackgroundColor() 
{
	return GetSysColor(COLOR_3DFACE);
}

// テキストの色
COLORREF SystemColorScheme::GetEditTextColor() 
{
	return GetSysColor(COLOR_WINDOWTEXT);
}

// 背景色の色
COLORREF SystemColorScheme::GetEditBackgroundColor() 
{
	return GetSysColor(COLOR_WINDOW);
}

// 背景色のブラシ
HBRUSH SystemColorScheme::GetEditBackgroundBrush() 
{
	COLORREF cr = GetEditBackgroundColor();
	if (in->mEditBkBrush != (HBRUSH)nullptr && cr == in->mEditBkColor) {
		return in->mEditBkBrush;
	}
	in->mEditBkColor = cr;
	in->mEditBkBrush.DeleteObject();
	in->mEditBkBrush.CreateSolidBrush(cr);
	return in->mEditBkBrush;
}

// キャレット
COLORREF SystemColorScheme::GetCaretColor(bool isIMEOn) 
{
	return isIMEOn ? RGB(255-160, 255-32, 255-240) : RGB(255, 255, 255);
}

// テキストの色
COLORREF SystemColorScheme::GetListTextColor() 
{
	return GetSysColor(COLOR_WINDOWTEXT);
}

// 選択項目のテキストの色
COLORREF SystemColorScheme::GetListHighlightTextColor() 
{
	return GetSysColor(COLOR_HIGHLIGHTTEXT);
}

// 背景色
HBRUSH SystemColorScheme::GetListBackgroundBrush() 
{
	COLORREF crBk = GetListBackgroundColor();
	if (in->mListBackgroundBrush != (HBRUSH)nullptr && crBk == in->mListBackgroundColor) {
		return in->mListBackgroundBrush;
	}

	in->mListBackgroundColor = crBk;
	in->mListBackgroundBrush.DeleteObject();
	in->mListBackgroundBrush.CreateSolidBrush(crBk);
	return in->mListBackgroundBrush;
}

COLORREF SystemColorScheme::GetListBackgroundColor()
{
	return GetSysColor(COLOR_WINDOW);
}

// 背景色(交互)
HBRUSH SystemColorScheme::GetListBackgroundAltBrush() 
{
	COLORREF crBk = GetSysColor(COLOR_WINDOW);

	if (in->mListBackgroundAltBrush != (HBRUSH)nullptr && crBk == in->mListBackgroundAltColor) {
		return in->mListBackgroundAltBrush;
	}
	in->mListBackgroundAltColor = crBk;

	COLORREF crBkAlt = GetListBackgroundAltColor();
	in->mListBackgroundAltBrush.DeleteObject();
	in->mListBackgroundAltBrush.CreateSolidBrush(crBkAlt);
	return in->mListBackgroundAltBrush;
}

COLORREF SystemColorScheme::GetListBackgroundAltColor()
{
	COLORREF crBk = GetSysColor(COLOR_WINDOW);

	BYTE rgb[] = { GetRValue(crBk), GetGValue(crBk), GetBValue(crBk) };

// 背景色を行単位で交互に変えるときの一方の色を決める
	// (基準とする色から6%ほど弱めた感じにしてみる)
	if (*(std::max_element(rgb, rgb+3)) > 128) {
		// 明るい寄り(というかたいてい白のはず..)の場合は黒方向に近づける
		rgb[0] = BYTE(rgb[0] * 0.94);
		rgb[1] = BYTE(rgb[1] * 0.94);
		rgb[2] = BYTE(rgb[2] * 0.94);
	}
	else {
		// 暗い寄り(ハイコントラストモードで動いている場合とか..)の場合は白方向に近づける
		rgb[0] = BYTE(rgb[0] + BYTE((255 - rgb[0]) * 0.06));
		rgb[1] = BYTE(rgb[1] + BYTE((255 - rgb[1]) * 0.06));
		rgb[2] = BYTE(rgb[2] + BYTE((255 - rgb[2]) * 0.06));
	}

	return RGB(rgb[0], rgb[1], rgb[2]);
}

// 選択項目の背景の色
HBRUSH SystemColorScheme::GetListHighlightBackgroundBrush() 
{
	COLORREF cr = GetListHighlightBackgroundColor();
	if (in->mListHighlightBackgroundBrush != (HBRUSH)nullptr && cr == in->mListHighlightBackgroundColor) {
		return in->mListHighlightBackgroundBrush;
	}

	in->mListHighlightBackgroundColor = cr;
	in->mListHighlightBackgroundBrush.DeleteObject();
	in->mListHighlightBackgroundBrush.CreateSolidBrush(cr);
	return in->mListHighlightBackgroundBrush; 
}

COLORREF SystemColorScheme::GetListHighlightBackgroundColor()
{
	return ::GetSysColor(COLOR_HIGHLIGHT);
}
