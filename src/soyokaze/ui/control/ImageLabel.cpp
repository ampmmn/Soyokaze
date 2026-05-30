#include "pch.h"
#include "ImageLabel.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ImageLabel::ImageLabel()
{
}

ImageLabel::~ImageLabel()
{
}

CBitmap* ImageLabel::GetBitmap()
{
	CClientDC dc(this);

	CRect rc;
	GetClientRect(&rc);

	// 初回またはサイズが変わってたらビットマップ作り直し
	CBitmap& memBmp{mBuffer};
	if (memBmp == (HBITMAP)nullptr || memBmp.GetBitmapDimension() != rc.Size()) {

		if (memBmp != (HBITMAP)nullptr) {
			memBmp.DeleteObject();
		}

		memBmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	}
	return &mBuffer;
}

BEGIN_MESSAGE_MAP(ImageLabel, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()


void ImageLabel::OnPaint()
{
	CPaintDC dc(this);

	CBitmap& bmp = mBuffer;
	if  (bmp == (HBITMAP)nullptr) {
		return;
	}

	CRect rc;
	GetClientRect(rc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	ASSERT(dcMem.GetSafeHdc() != NULL);

	CBitmap* orgBmp = dcMem.SelectObject(&bmp);
	dc.BitBlt(0,0, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY);
	dcMem.SelectObject(orgBmp);
}


