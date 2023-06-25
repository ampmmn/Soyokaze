#include "pch.h"
#include "framework.h"
#include "SettingDialogBase.h"
#include "SettingPage.h"
#include "utility/TopMostMask.h"
#include "resource.h"
#include <algorithm>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct SettingDialogBase::PImpl
{
	CBrush brBk;
	CTreeCtrl* mTreeCtrl;

	CRect mPageRect;

	// 最後に選択したツリー項目
	HTREEITEM mLastTreeItem;

	// ページ階層を示す文字列
	CString mBreadCrumbs;

	std::vector<SettingPage*> mPages;

	TopMostMask mTopMostMask;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



/**
	コンストラクタ
*/
 SettingDialogBase::SettingDialogBase() :  CDialogEx(IDD_SETTING),
	in(new PImpl)
{
	in->mTreeCtrl = nullptr;
	in->mLastTreeItem = nullptr;
}

/**
 	デストラクタ
*/
 SettingDialogBase::~SettingDialogBase()
{
	for (auto page : in->mPages) {
		delete page;
	}
	delete in;
}

void SettingDialogBase::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_BREADCRUMBS, in->mBreadCrumbs);
}

BEGIN_MESSAGE_MAP(SettingDialogBase, CDialogEx)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PAGES, OnTvnSelChangingPage)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// ダイアログ初期化
BOOL SettingDialogBase::OnInitDialog()
{
	__super::OnInitDialog();

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
	for (auto page : in->mPages) {
		page->OnEnterSettings();
		page->OnSetActive();
	}

	SelectPage(hItem);

//	in->mTreeCtrl->SetFocus();

	UpdateData(FALSE);

	return TRUE;
}

void SettingDialogBase::OnOK()
{
	for (auto page : in->mPages) {
		if (page->OnKillActive() == FALSE) {
			return;
		}
	}

	// 各プロパティページのOnOKをよぶ
	for (auto page : in->mPages) {
		page->OnOK();
	}

	__super::OnOK();

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
	std::vector<CString> pageNames;
	HTREEITEM h = hTreeItem;
	while(h) {
		SettingPage* newPagePtr = (SettingPage*)tree->GetItemData(h);

		const CString& str = newPagePtr->GetName();
		pageNames.push_back(str);

		h = tree->GetParentItem(h);
	}

	in->mBreadCrumbs.Empty();
	std::reverse(pageNames.begin(), pageNames.end());
	for (auto& name : pageNames) {
		if (in->mBreadCrumbs.IsEmpty() == FALSE) {
			in->mBreadCrumbs += _T(">");
		}
		in->mBreadCrumbs += _T(" ");
		in->mBreadCrumbs += name;
		in->mBreadCrumbs += _T(" ");
	}

	UpdateData(FALSE);

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
 	SettingPage* page,
	void* param
)
{
	ASSERT(page);

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
	in->mTreeCtrl->SetItemData(hItem, (DWORD_PTR)page);

	in->mPages.push_back(page);

	return hItem;

}

