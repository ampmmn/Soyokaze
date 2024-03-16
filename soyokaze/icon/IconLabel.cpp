#include "pch.h"
#include "framework.h"
#include "IconLabel.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IconLabel::IconLabel() : mIconDefault(nullptr), mCanIconChange(false)
{
}

IconLabel::~IconLabel()
{
}

BEGIN_MESSAGE_MAP(IconLabel, CStatic)
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void IconLabel::EnableIconChange()
{
	mCanIconChange = true;
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

void IconLabel::OnMenuChangeIcon()
{
	CString filterStr((LPCTSTR)IDS_FILTER_ICONIMAGEFILES);
	CString iconPath;
	LPTSTR p = iconPath.GetBuffer(MAX_PATH_NTFS);
	PathRemoveFileSpec(p);
	iconPath.ReleaseBuffer();
	GetModuleFileName(NULL, p, MAX_PATH_NTFS);

	CFileDialog dlg(TRUE, NULL, iconPath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	iconPath = dlg.GetPathName();

	// 親ウインドウに変更後のアイコントとする画像ファイルパスを通知する
	GetParent()->SendMessage(WM_APP + 11, 1, (LPARAM)(LPCTSTR)iconPath);

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
	if (mCanIconChange == false) {
		return;
	}

	const int ID_CHANGEICON = 1;
	const int ID_DEFAULTICON = 2;

	CMenu menu;
	menu.CreatePopupMenu();
	menu.InsertMenu(-1, 0, ID_CHANGEICON, _T("アイコンを変更する"));
	menu.InsertMenu(-1, 0, ID_DEFAULTICON, _T("アイコンを初期状態に戻す"));

	int n = menu.TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);
	if (n == ID_CHANGEICON) {
		OnMenuChangeIcon();
	}
	else if (n == ID_DEFAULTICON) {
		OnMenuDefaultIcon();
	}

}

