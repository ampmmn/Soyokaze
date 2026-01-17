#include "pch.h"
#include "framework.h"
#include "SettingDialogBase.h"
#include "SettingPage.h"
#include "app/Manual.h"
#include "control/BreadCrumbs.h"
#include "utility/TopMostMask.h"
#include "utility/Accessibility.h"
#include "resource.h"
#include <algorithm>
#include <vector>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct SettingDialogBase::PImpl
{
	CBrush brBk;
	CTreeCtrl* mTreeCtrl{nullptr};

	CRect mPageRect;

	// 最後に選択したツリー項目
	HTREEITEM mLastTreeItem{nullptr};

	// ページ階層を示す文字列
	CString mBreadCrumbs;

	std::vector<std::unique_ptr<SettingPage> > mPages;

	// OKできない状態のページの一覧
	std::set<SettingPage*> mInvalidPages;

	TopMostMask mTopMostMask;

	HACCEL mAccel{nullptr};


};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



/**
	コンストラクタ
*/
 SettingDialogBase::SettingDialogBase(CWnd* parentWnd) :  CDialogEx(IDD_SETTING, parentWnd),
	in(std::make_unique<PImpl>())
{
	ACCEL accels[1];
	accels[0].cmd = ID_VIEW_HELP;
	accels[0].fVirt = FVIRTKEY;
	accels[0].key = 0x70;   // F1
	in->mAccel = CreateAcceleratorTable(accels, 1);
}

/**
 	デストラクタ
*/
SettingDialogBase::~SettingDialogBase()
{
	if (in->mAccel) {
		DestroyAcceleratorTable(in->mAccel);
	}
}

CString SettingDialogBase::GetBreadCrumbsString()
{
	return in->mBreadCrumbs;
}

void SettingDialogBase::SetBreadCrumbsString(const CString& crumbs)
{
	in->mBreadCrumbs = crumbs;
}

void SettingDialogBase::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_BREADCRUMBS, in->mBreadCrumbs);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(SettingDialogBase, CDialogEx)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PAGES, OnTvnSelChangingPage)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_APP+1, OnUserEnableOKButton)
	ON_MESSAGE(WM_APP+2, OnUserDisableOKButton)
	ON_WM_NCLBUTTONDOWN()
	ON_COMMAND(ID_VIEW_HELP, OnCommandHelp)
END_MESSAGE_MAP()

#pragma warning( pop )

// ダイアログ初期化
BOOL SettingDialogBase::OnInitDialog()
{
	__super::OnInitDialog();

	ModifyStyleEx(0, WS_EX_CONTEXTHELP);

	in->brBk.CreateSolidBrush(RGB(0xBC, 0xE1, 0xDF));

	// 左側のツリーコントロール
	in->mTreeCtrl = (CTreeCtrl*)GetDlgItem(IDC_TREE_PAGES);
	in->mTreeCtrl->ModifyStyle(0, TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS);
	ASSERT(in->mTreeCtrl);

	// 右側のページ表示領域を記憶しておく
	GetDlgItem(IDC_STATIC_PAGEAREA)->GetClientRect(&in->mPageRect);
	GetDlgItem(IDC_STATIC_PAGEAREA)->MapWindowPoints(this, &in->mPageRect);

	// このタイミングで各ページを追加する
	// (派生クラス側で実施する)
	HTREEITEM hItem = OnSetupPages();

	// 各ページの設定をロードする
	for (auto& page : in->mPages) {
		page->OnEnterSettings();
		page->OnSetActive();
	}

	// もしパンくずリストが設定済なら、その状態を優先して表示する
	if (in->mBreadCrumbs.IsEmpty() == FALSE) {
		BreadCrumbs crumbs(in->mBreadCrumbs);
		HTREEITEM h = crumbs.FindTreeItem(in->mTreeCtrl);
		if (h) {
			hItem = h;
		}
	}

	SelectPage(hItem);

//	in->mTreeCtrl->SetFocus();

	UpdateData(FALSE);

	return TRUE;
}

void SettingDialogBase::OnOK()
{
	for (auto& page : in->mPages) {
		if (page->OnKillActive() == FALSE) {
			return;
		}
	}

	// 各プロパティページのOnOKをよぶ
	for (auto& page : in->mPages) {
		page->OnOK();
	}

	__super::OnOK();

}

BOOL SettingDialogBase::PreTranslateMessage(MSG* pMsg)
{
	if (in->mAccel && TranslateAccelerator(GetSafeHwnd(), in->mAccel, pMsg)) {
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}


bool SettingDialogBase::SelectPage(HTREEITEM hTreeItem)
{
	if (hTreeItem == in->mLastTreeItem) {
		return true;
	}

	CTreeCtrl* tree = in->mTreeCtrl;
	ASSERT(tree);

	CPropertyPage* newPagePtr = (CPropertyPage*)tree->GetItemData(hTreeItem);
	if (newPagePtr == nullptr) {
		return false;
	}

	// 古いページを選択解除する
	if (in->mLastTreeItem) {
		CPropertyPage* oldPagePtr = (CPropertyPage*)tree->GetItemData(in->mLastTreeItem);
		if (oldPagePtr) {
			if (oldPagePtr->OnKillActive() == FALSE) {
				return false;
			}
			oldPagePtr->ShowWindow(SW_HIDE);
		}
	}

	// 新しいページを選択する
	newPagePtr->OnSetActive();
	newPagePtr->ShowWindow(SW_SHOW);
	newPagePtr->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	tree->SelectItem(hTreeItem);
	in->mLastTreeItem = hTreeItem;

	// パンくずリスト更新
	BreadCrumbs crumbs(tree, hTreeItem);
	in->mBreadCrumbs = crumbs.ToString();

	UpdateData(FALSE);

	return true;
}

bool SettingDialogBase::ShowHelp()
{
	CTreeCtrl* tree = in->mTreeCtrl;
	ASSERT(tree);

	SettingPage* newPagePtr = (SettingPage*)tree->GetItemData(in->mLastTreeItem);
	if (newPagePtr == nullptr) {
		return false;
	}

	String helpPageId;
	if (newPagePtr->GetHelpPageId(helpPageId) == false) {
		return false;
	}

	auto manual{launcherapp::app::Manual::GetInstance()};
	manual->Navigate(helpPageId.c_str());
	return true;
}

/**
 *  ツリー要素の選択項目変更通知
 */
void SettingDialogBase::OnTvnSelChangingPage(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	LPNMTREEVIEW nmTreePtr = (LPNMTREEVIEW)pNMHDR;

	HTREEITEM hNewItem = nmTreePtr->itemNew.hItem;
	if( hNewItem == NULL ){
		return;
	}

	// 新しいページを表示
	SelectPage(hNewItem);

	// 再描画
	GetDlgItem(IDC_TREE_PAGES)->Invalidate();
	Invalidate();
}

HBRUSH SettingDialogBase::OnCtlColor(
	CDC* pDC,
	CWnd* pWnd,
	UINT nCtlColor
)
{
	if (::utility::IsHighContrastMode()) {
		HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_BREADCRUMBS) {
		COLORREF crBk = RGB(0xBC, 0xE1, 0xDF);
		pDC->SetBkColor(crBk);
		return in->brBk;
	}
	else {
		HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
		return br;
	}
}

HTREEITEM SettingDialogBase::AddPage(
	HTREEITEM parent,
 	std::unique_ptr<SettingPage>& page,
	void* param
)
{
	return AddPage(parent, std::move(page), param);
}

HTREEITEM SettingDialogBase::AddPage(
	HTREEITEM parent,
 	std::unique_ptr<SettingPage>&& page,
	void* param
)
{
	ASSERT(page.get());

	if (page->GetSafeHwnd() == NULL) {
		page->Create();
	}
	page->SetParam(param);

	CString name = page->GetName();
	HTREEITEM hItem = in->mTreeCtrl->InsertItem(name, parent);
	if (parent != TVI_ROOT) {
		in->mTreeCtrl->Expand(parent, TVE_EXPAND);
	}

	CRect& rcPage = in->mPageRect;
	page->MoveWindow(rcPage.left, rcPage.top, rcPage.Width(), rcPage.Height());
	in->mTreeCtrl->SetItemData(hItem, (DWORD_PTR)page.get());

	in->mPages.push_back(std::move(page));

	return hItem;

}

/**
 	OKボタンを有効化する
 	@return 0
 	@param[in] wp 0
 	@param[in] lp 通知元のページのポインタ
*/
LRESULT SettingDialogBase::OnUserEnableOKButton(
	WPARAM wp,
	LPARAM lp
)
{
	UNREFERENCED_PARAMETER(wp);

	SettingPage* page = (SettingPage*)lp;

	auto it = in->mInvalidPages.find(page);
	if (it == in->mInvalidPages.end()) {
		return 0;
	}

	in->mInvalidPages.erase(it);

	if (in->mInvalidPages.empty()) {
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}

	return 0;
}

/**
 	OKボタンを無効化する
 	@return 0
 	@param[in] wp 0
 	@param[in] lp 通知元のページのポインタ
*/
LRESULT SettingDialogBase::OnUserDisableOKButton(
	WPARAM wp,
	LPARAM lp
)
{
	UNREFERENCED_PARAMETER(wp);

	SettingPage* page = (SettingPage*)lp;
	in->mInvalidPages.insert(page);

	GetDlgItem(IDOK)->EnableWindow(FALSE);

	return 0;
}

void SettingDialogBase::OnNcLButtonDown(UINT nHitTest, CPoint pt)
{
	if (nHitTest == HTHELP) {
		ShowHelp();
	}
	else {
		__super::OnNcLButtonDown(nHitTest, pt);
	}
}

void SettingDialogBase::OnCommandHelp()
{
	ShowHelp();
}
