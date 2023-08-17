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
	CClientDC dc(this);
	DrawIcon(&dc, iconHandle);
}

void IconLabel::DrawIcon(CDC* pDC, HICON iconHandle)
{
	CRect rc;
	GetClientRect(rc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	ASSERT(dcMem.GetSafeHdc() != NULL);

	// 初回またはサイズが変わってたらビットマップ作り直し
	CBitmap& memBmp = mBuffer;
	if (memBmp == (HBITMAP)nullptr || memBmp.GetBitmapDimension() != rc.Size()) {

		if (memBmp != (HBITMAP)nullptr) {
			memBmp.DeleteObject();
		}

		memBmp.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
	}
	CBitmap* orgBmp = dcMem.SelectObject(&memBmp);

	CBrush br;
	br.CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	CBrush* orgBr = dcMem.SelectObject(&br);
	dcMem.PatBlt(0,0,rc.Width(), rc.Height(), PATCOPY);

	dcMem.DrawIcon(0, 0, iconHandle);

	pDC->BitBlt(0,0, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY);

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
	CPaintDC dc(this); // device context for painting

	CBitmap& bmp = mBuffer;
	if  (bmp == (HBITMAP)nullptr) {
		// 初回はデフォルトアイコンを描画
		mIconDefault = IconLoader::Get()->LoadDefaultIcon();
		DrawIcon(&dc, mIconDefault);
	}
	else {
		// 2回目以降は前回のバッファを使って描画
		CRect rc;
		GetClientRect(rc);

		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		ASSERT(dcMem.GetSafeHdc() != NULL);

		CBitmap* orgBmp = dcMem.SelectObject(&bmp);
		dc.BitBlt(0,0, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY);
		dcMem.SelectObject(orgBmp);
	}
}

