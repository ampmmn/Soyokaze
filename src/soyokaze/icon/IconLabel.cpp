#include "pch.h"
#include "framework.h"
#include "IconLabel.h"
#include "icon/IconLoader.h"
#include "utility/Path.h"
#include "resource.h"
#include <map>
#include <gdiplus.h>

#pragma comment(lib, "Gdiplus.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Gdiplus;

struct IconLabel::PImpl
{
	CBitmap mBuffer;
	std::map<HICON,int> mIconIndexMap;
	CImageList mIconList;
	CSize mCtrlSize;

	bool mCanIconChange{false};
	bool mIsUseBackgroundColor{false};
	COLORREF mBackgroundColor{RGB(0,0,0)};
};


IconLabel::IconLabel() : in(new PImpl)
{
}

IconLabel::~IconLabel()
{
}

BEGIN_MESSAGE_MAP(IconLabel, CStatic)
	ON_WM_ENABLE()
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void IconLabel::EnableIconChange()
{
	in->mCanIconChange = true;
}

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

	COLORREF crBr = in->mIsUseBackgroundColor == false ? GetSysColor(COLOR_3DFACE) : in->mBackgroundColor;

	// 初回またはサイズが変わってたらビットマップ作り直し
	CBitmap& memBmp{in->mBuffer};
	if (memBmp == (HBITMAP)nullptr || memBmp.GetBitmapDimension() != rc.Size()) {

		if (memBmp != (HBITMAP)nullptr) {
			memBmp.DeleteObject();
		}

		memBmp.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
	}

	auto& iconList = in->mIconList;
	auto& iconIndexMap = in->mIconIndexMap;
	auto& ctrlSize = in->mCtrlSize;

	CBitmap* orgBmp = dcMem.SelectObject(&memBmp);
	if (iconList.m_hImageList == nullptr || ctrlSize != rc.Size()) {
		iconList.DeleteImageList();
		iconList.Create(rc.Width(), rc.Height(), ILC_COLOR24 | ILC_MASK, 0, 0);
		iconList.SetBkColor(crBr);
		ctrlSize = rc.Size();
		iconIndexMap.clear();
	}

	int index = -1;

	auto it = iconIndexMap.find(iconHandle);
	if (it == iconIndexMap.end()) {
		index = iconList.Add(iconHandle);
		iconIndexMap[iconHandle] = index;
	}
	else {
		index = it->second;
	}
	CBrush br;

	br.CreateSolidBrush(crBr);
	CBrush* orgBr = dcMem.SelectObject(&br);
	dcMem.PatBlt(0,0,rc.Width(), rc.Height(), PATCOPY);

	if (index != -1) {
		iconList.DrawEx(&dcMem, index, CPoint(0, 0), ctrlSize, CLR_NONE,  CLR_DEFAULT, ILD_NORMAL);
		pDC->BitBlt(0, 0, ctrlSize.cx, ctrlSize.cy, &dcMem, 0, 0, SRCCOPY);
	}
	dcMem.SelectObject(orgBr);
	dcMem.SelectObject(orgBmp);
}

// デフォルトアイコンの描画
void IconLabel::DrawDefaultIcon()
{
	DrawIcon(IconLoader::Get()->LoadDefaultIcon());
}

void IconLabel::SetBackgroundColor(bool isUseSystemSetting, COLORREF cr)
{
	in->mIsUseBackgroundColor = !isUseSystemSetting;
	in->mBackgroundColor = cr;
}

void IconLabel::OnEnable(BOOL isEnable)
{
	InvalidateRect(nullptr);
}

void IconLabel::OnPaint()
{
	CRect rc;
	GetClientRect(rc);

	CPaintDC dc(this); // device context for painting

	CBitmap& bmp = in->mBuffer;
	if  (bmp == (HBITMAP)nullptr) {
		// 初回はデフォルトアイコンを描画
		DrawIcon(&dc, IconLoader::Get()->LoadDefaultIcon());
	}

	if (IsWindowEnabled()) {
		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		ASSERT(dcMem.GetSafeHdc() != NULL);

		CBitmap* orgBmp = dcMem.SelectObject(&bmp);
		dc.BitBlt(0,0, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY);
		dcMem.SelectObject(orgBmp);
	}
	else {
    Graphics g(dc.m_hDC);

    static ColorMatrix grayMatrix = {
        0.299f, 0.299f, 0.299f, 0, 0,
        0.587f, 0.587f, 0.587f, 0, 0,
        0.114f, 0.114f, 0.114f, 0, 0,
        0,      0,      0,      1, 0,
        0,      0,      0,      0, 1
    };

    ImageAttributes attr;
    attr.SetColorMatrix(&grayMatrix);

	Bitmap bmp2(bmp, (HPALETTE)NULL); 
    g.DrawImage(&bmp2, Rect(0, 0, bmp2.GetWidth(), bmp2.GetHeight()),
                0, 0, bmp2.GetWidth(), bmp2.GetHeight(),
                UnitPixel, &attr);
	}
}

void IconLabel::OnMenuChangeIcon()
{
	CString filterStr((LPCTSTR)IDS_FILTER_ICONIMAGEFILES);

	Path iconPath(Path::MODULEFILEPATH);
	iconPath.Shrink();

	CFileDialog dlg(TRUE, NULL, iconPath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	// 親ウインドウに変更後のアイコントとする画像ファイルパスを通知する
	GetParent()->SendMessage(WM_APP + 11, 1, (LPARAM)(LPCTSTR)dlg.GetPathName());

	// IconLabelに対してアイコンを設定するのはクラス利用者側の責務

}

void IconLabel::OnMenuDefaultIcon()
{
	// wparam=0でリセット
	GetParent()->SendMessage(WM_APP + 11, 0, 0);
}

/**
 * コンテキストメニューの表示
 */
void IconLabel::OnContextMenu(
	CWnd* pWnd,
	CPoint point
)
{
	UNREFERENCED_PARAMETER(pWnd);

	if (in->mCanIconChange == false) {
		return;
	}

	const int ID_CHANGEICON = 1;
	const int ID_DEFAULTICON = 2;

	CMenu menu;
	menu.CreatePopupMenu();
	menu.InsertMenu((UINT)-1, 0, ID_CHANGEICON, _T("アイコンを変更する"));
	menu.InsertMenu((UINT)-1, 0, ID_DEFAULTICON, _T("アイコンを初期状態に戻す"));

	int n = menu.TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);
	if (n == ID_CHANGEICON) {
		OnMenuChangeIcon();
	}
	else if (n == ID_DEFAULTICON) {
		OnMenuDefaultIcon();
	}

}

