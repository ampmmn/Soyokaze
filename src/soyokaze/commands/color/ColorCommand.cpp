#include "pch.h"
#include "framework.h"
#include "ColorCommand.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "commands/color/HSL.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using namespace launcherapp::actions::clipboard;

namespace launcherapp {
namespace commands {
namespace color {

// カラーアイコンのサイズ
constexpr int ICON_SIZE = 64;
constexpr int ICON_MARGIN = 8;
constexpr int ROUND_SIZE = 8;


struct ColorCommand::PImpl
{
	void CreateColorIcon(COLORREF cr);

	HICON mColorIcon{nullptr};
	COLORREF mColor{RGB(0, 0, 0)};
	int mFormatType{TYPE_HEX6};
};

void ColorCommand::PImpl::CreateColorIcon(COLORREF cr)
{
	// カラービットマップ作成
	ATL::CImage colorBmp;
	colorBmp.Create(ICON_SIZE, ICON_SIZE, 24);

	// カラービットマップの描画
	HDC bmpDC = colorBmp.GetDC();

	HBRUSH br = CreateSolidBrush(cr);
	HGDIOBJ hOrgBr = SelectObject(bmpDC, br);

	HPEN pen = (HPEN)CreatePen(PS_SOLID, 1, cr);
	auto hOrgPen = SelectObject(bmpDC, pen);

	PatBlt(bmpDC, 0, 0, ICON_SIZE, ICON_SIZE, BLACKNESS);
	RoundRect(bmpDC, ICON_MARGIN, ICON_MARGIN, ICON_SIZE-ICON_MARGIN, ICON_SIZE-ICON_MARGIN, ROUND_SIZE, ROUND_SIZE);
	SelectObject(bmpDC, hOrgPen);
	DeleteObject(pen);
	SelectObject(bmpDC, hOrgBr);
	DeleteObject(br);

	colorBmp.ReleaseDC();

	// マスクビットマップ作成
	SharedHwnd sharedWnd;
	HDC hdc = GetDC(sharedWnd.GetHwnd());
	HBITMAP maskBmp = CreateCompatibleBitmap(hdc, ICON_SIZE, ICON_SIZE);
	HDC memDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOrgBmp = SelectObject(memDC, maskBmp);

	struct BGR {
		BYTE b;
		BYTE g;
		BYTE r;
	};

	LPBYTE head = (LPBYTE)colorBmp.GetBits();
	int w = colorBmp.GetWidth();
	int h = colorBmp.GetHeight();
	for (int y = 0; y < h; ++y) {
		BGR* p = (BGR*)(head + colorBmp.GetPitch() * y);
		for (int x = 0; x < w; ++x) {
			bool isBGPixel = RGB(p[x].r, p[x].g, p[x].b) != cr;
			SetPixel(memDC, x, y, isBGPixel ? RGB(255, 255, 255) : RGB(0, 0, 0));
		}
	}

	SelectObject(memDC, hOrgBmp);
	DeleteDC(memDC);

	ReleaseDC(sharedWnd.GetHwnd(), hdc);

	// アイコン作成
	ICONINFO iconInfo;
	iconInfo.fIcon = TRUE;
	iconInfo.hbmMask = maskBmp;
	iconInfo.hbmColor = (HBITMAP)colorBmp;

	mColorIcon = CreateIconIndirect(&iconInfo);

	DeleteObject(maskBmp);
}


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
	in->CreateColorIcon(cr);
}

ColorCommand::~ColorCommand()
{
	if (in->mColorIcon) {
		DestroyIcon(in->mColorIcon);
	}
}

CString ColorCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool ColorCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags != 0) {
		return false;
	}
	// クリップボードにコピー
	*action = new CopyTextAction(mName);

	return true;
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

CString ColorCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_COLOR);
	return TEXT_TYPE;
}

} // end of namespace color
} // end of namespace commands
} // end of namespace launcherapp

