#include "pch.h"
#include "framework.h"
#include "IconLabel.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ToDo: アイコン表示用のコントロール作成

IconLabel::IconLabel()
{
}

IconLabel::~IconLabel()
{
}

BEGIN_MESSAGE_MAP(IconLabel, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void IconLabel::DrawIcon(HICON iconHandle)
{
	CRect rc;
	GetClientRect(rc);

	CClientDC dc(this);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	ASSERT(dcMem.GetSafeHdc() != NULL);

	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* orgBmp = dcMem.SelectObject(&memBmp);

	CBrush br;
	br.CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	CBrush* orgBr = dcMem.SelectObject(&br);
	dcMem.PatBlt(0,0,rc.Width(), rc.Height(), PATCOPY);

	dcMem.DrawIcon(0, 0, iconHandle);

	dc.BitBlt(0,0, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject(orgBr);
	dcMem.SelectObject(orgBmp);
}

// デフォルトアイコンの描画
void IconLabel::DrawDefaultIcon()
{
	if (mIconDefault == nullptr) {
		mIconDefault = IconLoader::Get()->LoadDefaultIcon();
	}

	DrawIcon(mIconDefault);
}

void IconLabel::OnPaint()
{
	CRect rc;
	GetClientRect(rc);

	CPaintDC dc(this); // device context for painting

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* orgBmp = dcMem.SelectObject(&bmp);

	CBrush br;
	br.CreateSolidBrush(GetSysColor(COLOR_3DFACE));

	CBrush* orgBr = dcMem.SelectObject(&br);
	dcMem.PatBlt(0,0,rc.Width(), rc.Height(), PATCOPY);

	mIconDefault = IconLoader::Get()->LoadDefaultIcon();
	dcMem.DrawIcon(0, 0, mIconDefault);

	dc.BitBlt(0,0, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject(orgBr);
	dcMem.SelectObject(orgBmp);
}

