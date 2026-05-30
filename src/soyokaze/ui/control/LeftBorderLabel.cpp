#include "pch.h"
#include "LeftBorderLabel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct LeftBorderLabel::PImpl
{
	HWND mHwnd{nullptr};
	ATL::CImage mBuffer;
	COLORREF mBorderColor{RGB(0, 120, 212)};

	int mBorderWidth{4};
	int mBorderMargin{10};
	bool mIsTextColorSameAsBorder{true};
};

namespace {

bool isInitialized = false;

}

LeftBorderLabel::LeftBorderLabel() : in(new PImpl)
{
}

LeftBorderLabel::~LeftBorderLabel()
{
}

bool LeftBorderLabel::Initialize()
{
	if (isInitialized) {
		return true;
	}

	WNDCLASSEX wc = { 0 };
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = LeftBorderLabel::WindowProc;
	wc.hInstance     = GetModuleHandle(nullptr);
	wc.hIcon         = nullptr;
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"LeftBorderLabel";

	if (RegisterClassEx(&wc) == FALSE) {
		spdlog::error("Failed to regsiter LeftBorderLabel class.");
		return false;
	}

	isInitialized = true;
	return true;
}


void LeftBorderLabel::Attach(HWND h)
{
	in->mHwnd = h;
	SetWindowLongPtr(h, GWLP_USERDATA, (LONG_PTR)this);
}

void LeftBorderLabel::SetBorderWidth(int cx)
{
	in->mBorderWidth = cx;
}

void LeftBorderLabel::SetBorderMargin(int cx)
{
	in->mBorderMargin = cx;
}

void LeftBorderLabel::SetBorderColor(COLORREF cr)
{
	in->mBorderColor = cr;
}

void LeftBorderLabel::Draw()
{
	// サイズ決定
	CRect rc;
	GetClientRect(in->mHwnd, &rc);
	CSize border(in->mBorderWidth, rc.Height());

	CRect rcItem(border.cx + in->mBorderMargin, 0, rc.right, rc.bottom);

	COLORREF crBk = GetSysColor(COLOR_3DFACE);
	BYTE rgb[] = { GetRValue(crBk), GetGValue(crBk), GetBValue(crBk) };
	rgb[0] = BYTE(rgb[0] * 0.98);
	rgb[1] = BYTE(rgb[1] * 0.98);
	rgb[2] = BYTE(rgb[2] * 0.98);

	CBrush brBk;
	brBk.CreateSolidBrush(RGB(rgb[0], rgb[1], rgb[2]));

	CBrush brBorder;
	brBorder.CreateSolidBrush(in->mBorderColor);

	CDC dc;
	dc.Attach(in->mBuffer.GetDC());

	// 背景色を描画
	auto orgBr = dc.SelectObject(&brBk);
	dc.PatBlt(0, 0, rc.Width(), rc.Height(), PATCOPY);

	// 左側のボーダー色を描画
	dc.SelectObject(&brBorder);
	dc.PatBlt(0, 0, border.cx, border.cy, PATCOPY);

	// テキストを描画
	wchar_t text[256];
	GetWindowTextW(in->mHwnd, text, 256);

	int orgMode = dc.SetBkMode(TRANSPARENT);
	dc.SelectObject(orgBr);

	CFont* currentFont = CWnd::FromHandle(GetParent(in->mHwnd))->GetFont();

	LOGFONT lf;
	currentFont->GetLogFont(&lf);
	lf.lfWeight = 700;
	HFONT hf = CreateFontIndirect(&lf);

	auto orgFont = dc.SelectObject(hf);

	if (in->mIsTextColorSameAsBorder) {
		int textColor = dc.SetTextColor(in->mBorderColor);
		dc.DrawText(text, rcItem, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);
		dc.SetTextColor(textColor);
	}
	else {
		dc.DrawText(text, rcItem, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP);
	}

	dc.SelectObject(orgFont);
	dc.SetBkMode(orgMode);
	dc.Detach();
	in->mBuffer.ReleaseDC();

	DeleteObject(hf);

}

void LeftBorderLabel::OnPaint()
{
	CRect rc;
	GetClientRect(in->mHwnd, &rc);

	if ((HBITMAP)in->mBuffer == nullptr) {
		in->mBuffer.Create(rc.Width(), rc.Height(), 24);
		Draw();
	}

	CPaintDC dc(CWnd::FromHandle(in->mHwnd));
	CDC mdc;
	mdc.Attach(in->mBuffer.GetDC());
	dc.BitBlt(0, 0, rc.Width(), rc.Height(), &mdc, 0, 0, SRCCOPY);
	mdc.Detach();
	in->mBuffer.ReleaseDC();
}

void LeftBorderLabel::OnSize(UINT type, int cx, int cy)
{
	if (cx == 0 || cy == 0) {
		return;
	}

	if ((HBITMAP)in->mBuffer != nullptr) {
		in->mBuffer.Destroy();
	}

	in->mBuffer.Create(cx, cy, 24);

	Draw();
}

LRESULT CALLBACK LeftBorderLabel::WindowProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
	auto thisPtr = (LeftBorderLabel*)GetWindowLongPtr(h, GWLP_USERDATA);
	if (thisPtr == nullptr) {
		return ::DefWindowProc(h, msg, wp, lp);
	}

	if (msg == WM_PAINT) {
		thisPtr->OnPaint();
		return 0;
	}
	else if (msg == WM_SIZE) {
		thisPtr->OnSize((int)wp, (int)LOWORD(lp), (int)HIWORD(lp));
		return 0;
	}

	return ::DefWindowProc(h, msg, wp, lp);
}

