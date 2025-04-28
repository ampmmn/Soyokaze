#include "pch.h"
#include "framework.h"
#include "ColorCommand.h"
#include "commands/color/HSL.h"
#include "icon/IconLoader.h"
#include "commands/common/Clipboard.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace color {

struct ColorCommand::PImpl
{
	HICON mColorIcon{nullptr};
	COLORREF mColor{RGB(0, 0, 0)};
	int mFormatType{TYPE_HEX6};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(ColorCommand)

ColorCommand::ColorCommand(COLORREF cr, int formatType) : in(std::make_unique<PImpl>())
{
	in->mColor = cr;

	BYTE r = GetRValue(cr);
	BYTE g = GetGValue(cr);
	BYTE b = GetBValue(cr);

	CString name;

	if (formatType == TYPE_HEX6) {
		name.Format(_T("#%02X%02X%02X"), r, g, b);
	}
	else if (formatType == TYPE_HEX3) {
		name.Format(_T("#%02X%02X%02X"), r, g, b);
		LPTSTR p = name.GetBuffer(8);
		p[2] = p[3];
		p[3] = p[5];
		name.ReleaseBuffer();
		name = name.Left(4);
	}
	else if (formatType == TYPE_RGB) {
		name.Format(_T("rgb(%d,%d,%d)"), r, g, b);
	}
	else if (formatType == TYPE_HSL) {
		// hue
		HSL hsl;
		hsl.FromRGB(r, g, b);
		name.Format(_T("hsl(%d,%d%%,%d%%)"), 
		            hsl.H(), (int)std::round(hsl.S() * 100), (int)std::round(hsl.L() * 100));
	}

	this->mName = name;
	this->mDescription = this->mName;

	in->mFormatType = formatType;

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
}

ColorCommand::~ColorCommand()
{
	if (in->mColorIcon) {
		DestroyIcon(in->mColorIcon);
	}
}

CString ColorCommand::GetGuideString()
{
	return _T("⏎:クリップボードにコピー");
}

CString ColorCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_COLOR);
	return TEXT_TYPE;
}

BOOL ColorCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// クリップボードにコピー
	Clipboard::Copy(mName);
	return TRUE;
}


HICON ColorCommand::GetIcon()
{
	return in->mColorIcon;
}

launcherapp::core::Command*
ColorCommand::Clone()
{
	return new ColorCommand(in->mColor, in->mFormatType);
}

} // end of namespace color
} // end of namespace commands
} // end of namespace launcherapp

