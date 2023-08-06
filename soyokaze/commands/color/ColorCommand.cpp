#include "pch.h"
#include "framework.h"
#include "ColorCommand.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace color {


struct ColorCommand::PImpl
{
	HICON mColorIcon;
	CString mName;
	CString mDescription;
	COLORREF mColor;
	uint32_t mRefCount;
};


ColorCommand::ColorCommand(COLORREF cr) : in(new PImpl)
{
	in->mColor = cr;

	BYTE r = GetRValue(cr);
	BYTE g = GetGValue(cr);
	BYTE b = GetBValue(cr);

	in->mName.Format(_T("#%02X%02X%02X"), r, g, b);
	in->mDescription = in->mName;

	// カラーアイコンの作成
	SharedHwnd sharedWnd;
	HDC hdc = GetDC(sharedWnd.GetHwnd());

	HBITMAP maskBmp = CreateCompatibleBitmap(hdc, 0, 0);
	HBITMAP colorBmp = CreateCompatibleBitmap(hdc, 1, 1);

	HDC memDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOrgBmp = SelectObject(memDC, maskBmp);
	PatBlt(memDC, 0, 0, 1, 1, BLACKNESS);

	HBRUSH br = CreateSolidBrush(cr);

	SelectObject(memDC, colorBmp);
	HGDIOBJ hOrgBr = SelectObject(memDC, br);
	PatBlt(memDC, 0, 0, 1, 1, PATCOPY);

	SelectObject(memDC, hOrgBmp);
	SelectObject(memDC, hOrgBr);

	DeleteObject(br);
	DeleteDC(memDC);
	ReleaseDC(sharedWnd.GetHwnd(), hdc);

	ICONINFO iconInfo;
	iconInfo.fIcon = TRUE;
	iconInfo.hbmMask = maskBmp;
	iconInfo.hbmColor = colorBmp;

	// 
	in->mColorIcon = CreateIconIndirect(&iconInfo);

	DeleteObject(maskBmp);
	DeleteObject(colorBmp);

	in->mRefCount = 1;
}

ColorCommand::~ColorCommand()
{
	if (in->mColorIcon) {
		DestroyIcon(in->mColorIcon);
	}
}

CString ColorCommand::GetName()
{
	return in->mName;
}

CString ColorCommand::GetDescription()
{
	return in->mDescription;
}

CString ColorCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_COLOR);
	return TEXT_TYPE;
}

BOOL ColorCommand::Execute()
{
	Parameter emptyParams;
	return Execute(emptyParams);
}

BOOL ColorCommand::Execute(const Parameter& param)
{
	// することない...

	return TRUE;
}

CString ColorCommand::GetErrorString()
{
	return _T("");
}

HICON ColorCommand::GetIcon()
{
	return in->mColorIcon;
}

int ColorCommand::Match(Pattern* pattern)
{
	// サポートしない
	return Pattern::WholeMatch;
}

bool ColorCommand::IsEditable()
{
	return false;
}

int ColorCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
ColorCommand::Clone()
{
	auto clonedObj = new ColorCommand(in->mColor);
	return clonedObj;
}

bool ColorCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t ColorCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t ColorCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace color
} // end of namespace commands
} // end of namespace soyokaze

