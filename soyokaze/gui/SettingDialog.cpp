#include "pch.h"
#include "framework.h"
#include "SettingDialog.h"
#include "gui/ShortcutDialog.h"
#include "gui/BasicSettingDialog.h"
#include "gui/ExtensionSettingDialog.h"
#include "gui/ShortcutSettingPage.h"
#include "resource.h"
#include <algorithm>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct SettingDialog::PImpl
{
	HTREEITEM AddPage(HTREEITEM parent, SettingPage* page);

	// 設定情報
	Settings mSettings;

	CBrush brBk;
	CTreeCtrl* mTreeCtrl;

	CRect mPageRect;

	// 最後に選択したツリー項目
	HTREEITEM mLastTreeItem;

	// ページ階層を示す文字列
	CString mBreadCrumbs;

	std::vector<SettingPage*> mPages;

};

HTREEITEM SettingDialog::PImpl::AddPage(
	HTREEITEM parent,
 	SettingPage* page
)
{
	ASSERT(page);

	if (page->GetSafeHwnd() == NULL) {
		page->Create();
	}
	page->SetSettings(&mSettings);

	CString name = page->GetName();
	HTREEITEM hItem = mTreeCtrl->InsertItem(name, parent);
	if (parent != TVI_ROOT) {
		mTreeCtrl->Expand(parent, TVE_EXPAND);
	}

	page->MoveWindow(mPageRect.left, mPageRect.top, mPageRect.Width(), mPageRect.Height());
	mTreeCtrl->SetItemData(hItem, (DWORD_PTR)page);

	mPages.push_back(page);

	return hItem;

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog() :  CDialogEx(IDD_SETTING),
	in(new PImpl)
{
	in->mTreeCtrl = nullptr;
	in->mLastTreeItem = nullptr;
}

SettingDialog::~SettingDialog()
{
	for (auto page : in->mPages) {
		delete page;
	}
	delete in;
}

void SettingDialog::SetSettings(const Settings& settings)
{
	std::unique_ptr<Settings> tmp(settings.Clone());
	in->mSettings.Swap(*tmp.get());
}

const Settings& SettingDialog::GetSettings()
{
	return in->mSettings;
}


void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_BREADCRUMBS, in->mBreadCrumbs);
}

BEGIN_MESSAGE_MAP(SettingDialog, CDialogEx)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PAGES, OnTvnSelChangingPage)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
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


	// 各ページ作成
	HTREEITEM hItem = in->AddPage(TVI_ROOT, new BasicSettingDialog(this));
	in->AddPage(hItem, new ShortcutSettingPage(this));

	// ToDo: 拡張機能を実装するときに有効化する
	//in->AddPage(TVI_ROOT, new ExtensionSettingDialog(this));

	SelectPage(hItem);

//	in->mTreeCtrl->SetFocus();

	UpdateData(FALSE);

	return TRUE;
}

void SettingDialog::OnOK()
{

	// 各プロパティページのOnOKをよぶ
	for (auto page : in->mPages) {
		page->OnOK();;
	}

	__super::OnOK();

}


bool SettingDialog::SelectPage(HTREEITEM hTreeItem)
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
void SettingDialog::OnTvnSelChangingPage(NMHDR *pNMHDR, LRESULT *pResult)
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

HBRUSH SettingDialog::OnCtlColor(
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
