#include "pch.h"
#include "CustomColorScheme.h"
#include "SystemColorScheme.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct CustomColorScheme::PImpl
{
	SystemColorScheme mSysColor;

	CBrush mWindowBkBrush;
	CBrush mEditBkBrush;
	CBrush mListBackgroundBrush;
	CBrush mListBackgroundAltBrush;
	CBrush mListHighlightBackgroundBrush;

	COLORREF mWindowTextColor{0};
	COLORREF mWindowBkColor{0};
	// 入力欄
	COLORREF mEditTextColor{0};
	COLORREF mEditBkColor{0};

	// リスト
	COLORREF mListTextColor{0};
	COLORREF mListBackgroundColor{0};
	COLORREF mListBackgroundAltColor{0};
	COLORREF mListTextHighlightColor{0};
	COLORREF mListBackgroundHighlightColor{0};
};

CustomColorScheme::CustomColorScheme() : in(new PImpl)
{
	Reload();
}

CustomColorScheme::~CustomColorScheme() 
{
}

void CustomColorScheme::Reload()
{
	auto pref = AppPreference::Get();

	// キャッシュしたブラシを破棄
	in->mWindowBkBrush.DeleteObject();
	in->mEditBkBrush.DeleteObject();
	in->mListBackgroundBrush.DeleteObject();
	in->mListBackgroundAltBrush.DeleteObject();
	in->mListHighlightBackgroundBrush.DeleteObject();

	// ウインドウ
	in->mWindowTextColor = pref->GetWindowTextColor();
	in->mWindowBkColor = pref->GetWindowBackgroundColor();
	// 入力欄
	in->mEditTextColor = pref->GetEditTextColor();
	in->mEditBkColor = pref->GetEditBackgroundColor();

	// リスト
	in->mListTextColor = pref->GetListTextColor();
	in->mListBackgroundColor = pref->GetListBackgroundColor();
	in->mListBackgroundAltColor = pref->GetListBackgroundAltColor();
	in->mListTextHighlightColor = pref->GetListTextHighlightColor();
	in->mListBackgroundHighlightColor = pref->GetListBackgroundHighlightColor();
}

// ウインドウの背景ブラシ
HBRUSH CustomColorScheme::GetBackgroundBrush() 
{
	if (in->mWindowBkBrush != (HBRUSH)nullptr) {
		return in->mWindowBkBrush;
	}
	COLORREF cr = GetBackgroundColor(); 
	in->mWindowBkBrush.CreateSolidBrush(cr);
	return in->mWindowBkBrush;
}

// テキストの色
COLORREF CustomColorScheme::GetTextColor() 
{
	return in->mWindowTextColor;
}

// 背景色
COLORREF CustomColorScheme::GetBackgroundColor() 
{
	return in->mWindowBkColor;
}

// テキストの色
COLORREF CustomColorScheme::GetEditTextColor() 
{
	return in->mEditTextColor;
}

// 背景色の色
COLORREF CustomColorScheme::GetEditBackgroundColor() 
{
	return in->mEditBkColor;
}

// 背景色のブラシ
HBRUSH CustomColorScheme::GetEditBackgroundBrush() 
{
	if (in->mEditBkBrush != (HBRUSH)nullptr) {
		return in->mEditBkBrush;
	}
	COLORREF cr = GetEditBackgroundColor();
	in->mEditBkBrush.CreateSolidBrush(cr);
	return in->mEditBkBrush;
}

// キャレット
COLORREF CustomColorScheme::GetCaretColor(bool isIMEOn) 
{
	return in->mSysColor.GetCaretColor(isIMEOn);
}

// テキストの色
COLORREF CustomColorScheme::GetListTextColor() 
{
	return in->mListTextColor;
}

// 選択項目のテキストの色
COLORREF CustomColorScheme::GetListHighlightTextColor() 
{
	return in->mListTextHighlightColor;
}

// 背景色
HBRUSH CustomColorScheme::GetListBackgroundBrush() 
{
	if (in->mListBackgroundBrush != (HBRUSH)nullptr) {
		return in->mListBackgroundBrush;
	}

	COLORREF crBk = GetListBackgroundColor();
	in->mListBackgroundBrush.CreateSolidBrush(crBk);
	return in->mListBackgroundBrush;
}

COLORREF CustomColorScheme::GetListBackgroundColor()
{
	return in->mListBackgroundColor;
}

// 背景色(交互)
HBRUSH CustomColorScheme::GetListBackgroundAltBrush() 
{
	if (in->mListBackgroundAltBrush != (HBRUSH)nullptr) {
		return in->mListBackgroundAltBrush;
	}
	COLORREF crBkAlt = GetListBackgroundAltColor();
	in->mListBackgroundAltBrush.CreateSolidBrush(crBkAlt);
	return in->mListBackgroundAltBrush;
}

COLORREF CustomColorScheme::GetListBackgroundAltColor()
{
	return in->mListBackgroundAltColor;
}

// 選択項目の背景の色
HBRUSH CustomColorScheme::GetListHighlightBackgroundBrush() 
{
	if (in->mListHighlightBackgroundBrush != (HBRUSH)nullptr) {
		return in->mListHighlightBackgroundBrush;
	}

	COLORREF cr = GetListHighlightBackgroundColor();
	in->mListHighlightBackgroundBrush.CreateSolidBrush(cr);
	return in->mListHighlightBackgroundBrush; 
}

COLORREF CustomColorScheme::GetListHighlightBackgroundColor()
{
	return in->mListBackgroundHighlightColor;
}
