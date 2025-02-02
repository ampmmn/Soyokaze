#include "pch.h"
#include "framework.h"
#include "IconLabelForApp.h"
#include "icon/IconLoader.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IconLabelForApp::IconLabelForApp() :
 	mIconDefault(nullptr),
 	mCanIconChange(false),
 	mIsUseBackgroundColor(false),
 	mBackgroundColor(RGB(0,0,0))
{
}

IconLabelForApp::~IconLabelForApp()
{
}

BEGIN_MESSAGE_MAP(IconLabelForApp, CStatic)
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

void IconLabelForApp::EnableIconChange()
{
	mCanIconChange = true;
}

void IconLabelForApp::DisableIconChange()
{
	mCanIconChange = false;
}

void IconLabelForApp::DrawIcon(HICON iconHandle)
{
	CClientDC dc(this);
	DrawIcon(&dc, iconHandle);
}

void IconLabelForApp::DrawIcon(CDC* pDC, HICON iconHandle)
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

	COLORREF crBr = mIsUseBackgroundColor == false ? GetSysColor(COLOR_3DFACE) : mBackgroundColor;
	br.CreateSolidBrush(crBr);
	CBrush* orgBr = dcMem.SelectObject(&br);
	dcMem.PatBlt(0,0,rc.Width(), rc.Height(), PATCOPY);

	CSize sizeIcon(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
	CPoint offset((rc.Width() - sizeIcon.cx)/2, (rc.Height() - sizeIcon.cy)/2);
	dcMem.DrawIcon(offset.x, offset.y, iconHandle);

	pDC->BitBlt(0,0, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject(orgBr);
	dcMem.SelectObject(orgBmp);
}

// デフォルトアイコンの描画
void IconLabelForApp::DrawDefaultIcon()
{
	if (mIconDefault == nullptr) {
		// 取得したアイコンの所有権はIconLoader側にあるので解放不要
		mIconDefault = IconLoader::Get()->LoadDefaultIcon();
	}
	DrawIcon(mIconDefault);
}

void IconLabelForApp::SetBackgroundColor(bool isUseSystemSetting, COLORREF cr)
{
	mIsUseBackgroundColor = !isUseSystemSetting;
	mBackgroundColor = cr;
}

void IconLabelForApp::OnPaint()
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

void IconLabelForApp::OnMenuChangeIcon()
{
	CString filterStr((LPCTSTR)_T("icoファイル(*.ico)|*.ico||"));

	Path iconPath(Path::MODULEFILEPATH);
	iconPath.Shrink();

	CFileDialog dlg(TRUE, NULL, iconPath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	// 親ウインドウに変更後のアイコンとするファイルパスを通知する
	GetParent()->SendMessage(WM_APP + 11, 1, (LPARAM)(LPCTSTR)dlg.GetPathName());

	// IconLabelForAppに対してアイコンを設定するのはクラス利用者側の責務

}

void IconLabelForApp::OnMenuDefaultIcon()
{
	// wparam=0でリセット
	GetParent()->SendMessage(WM_APP + 11, 0, 0);
}

/**
 * コンテキストメニューの表示
 */
void IconLabelForApp::OnContextMenu(
	CWnd* pWnd,
	CPoint point
)
{
	UNREFERENCED_PARAMETER(pWnd);

	if (mCanIconChange == false) {
		return;
	}

	const int ID_CHANGEICON = 1;

	CMenu menu;
	menu.CreatePopupMenu();
	menu.InsertMenu((UINT)-1, 0, ID_CHANGEICON, _T("アイコンを変更する"));

	int n = menu.TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);
	if (n == ID_CHANGEICON) {
		OnMenuChangeIcon();
	}
}

void IconLabelForApp::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	__super::OnLButtonDblClk(nFlags, point);

	if (mCanIconChange == false) {
		return;
	}

	OnMenuChangeIcon();
}

