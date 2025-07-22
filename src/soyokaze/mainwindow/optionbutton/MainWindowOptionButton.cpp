#include "pch.h"
#include "MainWindowOptionButton.h"
#include "gui/ColorSettings.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr int TIMERID_UPDATESTATE = 1;

struct MainWindowOptionButton::PImpl
{
	HBITMAP AcquireBitmap()
	{
		return (HBITMAP)mBitmap;
	}

	bool IsIconInitialized() 
	{
		return mIsIconInitialized;
	}

	void GetIconSize(HICON icon, CSize& size)
	{
		ICONINFO iconInfo;
		GetIconInfo(icon, &iconInfo);

		BITMAP bmp;
		GetObject(iconInfo.hbmColor ? iconInfo.hbmColor : iconInfo.hbmMask, sizeof(BITMAP), &bmp);

		// マスクのみの場合、縦サイズが2倍になっているので調整
		if (!iconInfo.hbmColor) {
			bmp.bmHeight /= 2;
		}

		if (iconInfo.hbmColor) {
			DeleteObject(iconInfo.hbmColor);
		}
		if (iconInfo.hbmMask) {
			DeleteObject(iconInfo.hbmMask);
		}

		size.cx = bmp.bmWidth;
		size.cy = bmp.bmHeight;
	}

	void DrawIcon(CDC* dcMem, const CRect& rcItem)
	{
		if (mBitmap != (HBITMAP)nullptr) {
			mBitmap.DeleteObject();
		}

		int itemW = rcItem.Width();
		int itemH = rcItem.Height();
		mBitmap.CreateCompatibleBitmap(dcMem, itemW, itemH);

		auto oldBmp = dcMem->SelectObject(mBitmap);

		auto cs = ColorSettings::Get()->GetCurrentScheme();
		HBRUSH br = CreateSolidBrush(cs->GetBackgroundColor());

		dcMem->FillRect(CRect(0, 0, itemW, itemH), CBrush::FromHandle(br));

		CSize sizeIcon;
		auto iconLoader = IconLoader::Get();
		HICON iconHandle = iconLoader->LoadSettingIcon();
		GetIconSize(iconHandle, sizeIcon);

		int len = (std::min)(itemW, itemH);
		int x = (len - itemW) / 2;
		int y = (len - itemH) / 2;
		DrawIconEx(*dcMem, x, y, iconHandle, len, len, 0, nullptr, DI_NORMAL);

		dcMem->SelectObject(oldBmp);

		mImageSize = CSize(itemW, itemH);
		mIsIconInitialized = true;
	}

	void UpdateState(CWnd* wnd, CPoint pt) {

		CRect rc;
		wnd->GetClientRect(&rc);

		bool oldState = mIsMouseOnWindow;
		mIsMouseOnWindow = PtInRect(&rc, pt) != FALSE;

		if (oldState != mIsMouseOnWindow) {
			// マウス位置がクライアント領域の内外にあるかどうかの状態が変化したとき、再描画を行う
			wnd->InvalidateRect(nullptr);
			wnd->UpdateWindow();
		}

	}

	CBitmap mBitmap;
	CSize mImageSize;
	bool mIsIconInitialized{false};
	bool mIsMouseOnWindow{false};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



MainWindowOptionButton::MainWindowOptionButton() : in(std::make_unique<PImpl>())
{
}

MainWindowOptionButton::~MainWindowOptionButton()
{
}

BEGIN_MESSAGE_MAP(MainWindowOptionButton, CButton)
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


void MainWindowOptionButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// 背景用のブラシ
	auto cs = ColorSettings::Get()->GetCurrentScheme();
	COLORREF bgColor = cs->GetBackgroundColor();

	if (in->mIsMouseOnWindow) {
		// マウスがボタン上にある場合、背景色を少し暗くする
		double r = GetRValue(bgColor) * 0.90;
		double g = GetGValue(bgColor) * 0.90;
		double b = GetBValue(bgColor) * 0.90;
		bgColor = RGB(((BYTE)r), ((BYTE)g), ((BYTE)b));
	}

	HBRUSH br = CreateSolidBrush(bgColor);

	CSize sizeIcon;
	auto iconLoader = IconLoader::Get();
	HICON iconHandle = iconLoader->LoadSettingIcon();
	in->GetIconSize(iconHandle, sizeIcon);

	CRect rcItem(lpDrawItemStruct->rcItem);
	int itemW = rcItem.Width();
	int itemH = rcItem.Height();

	// アイコンの描画サイズを決定する
	// 縦横の短辺に合わせる
	int iconDrawSize = (std::min)(itemW, itemH);
	// 短辺の80%サイズにする
	iconDrawSize = (int)(iconDrawSize * 0.8);
	// ただし、最低16pixel以上
	if (iconDrawSize < 16) { iconDrawSize = 16; }

	int x = (itemW - iconDrawSize) / 2;
	int y = (itemH - iconDrawSize) / 2;

	HDC dc = lpDrawItemStruct->hDC;

	auto oldBr = SelectObject(dc, br);
	PatBlt(dc, 0, 0, rcItem.Width(), rcItem.Height(), PATCOPY);
	SelectObject(dc, oldBr);

	int orgMode = SetStretchBltMode(dc, HALFTONE);
	DrawIconEx(dc, x, y, iconHandle, iconDrawSize, iconDrawSize, 0, nullptr, DI_NORMAL);
	SetStretchBltMode(dc, orgMode);

	DeleteObject(br);
}

void MainWindowOptionButton::OnMouseMove(UINT flags, CPoint pt)
{
	__super::OnMouseMove(flags, pt);

	in->UpdateState(this, pt);

	::SetTimer(GetSafeHwnd(), TIMERID_UPDATESTATE, 100, 0);
}

void MainWindowOptionButton::OnTimer(UINT_PTR timerId)
{
	if (timerId != TIMERID_UPDATESTATE) {
		return;
	}
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	in->UpdateState(this, pt);

	if (in->mIsMouseOnWindow == false) {
		KillTimer(TIMERID_UPDATESTATE);
	}
}
