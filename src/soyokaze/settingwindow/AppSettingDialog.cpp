#include "pch.h"
#include "framework.h"
#include "AppSettingDialog.h"
#include "commands/core/CommandRepository.h"
#include "settingwindow/AppSettingPageRepository.h"
#include "utility/TopMostMask.h"
#include "utility/Accessibility.h"
#include "app/Manual.h"
#include "gui/BreadCrumbs.h"
#include "resource.h"
#include <algorithm>
#include <vector>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using AppSettingPageIF = launcherapp::settingwindow::AppSettingPageIF;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct AppSettingDialog::PImpl
{
	HTREEITEM AddPage(HTREEITEM parent, AppSettingPageIF* page, void* param);

	CBrush brBk;
	CTreeCtrl* mTreeCtrl = nullptr;

	CRect mPageRect;

	// 最後に選択したツリー項目
	HTREEITEM mLastTreeItem = nullptr;

	// ページ階層を示す文字列
	CString mBreadCrumbs;

	std::vector<AppSettingPageIF*> mPages;

	// OKできない状態のページの一覧
	std::set<SettingPage*> mInvalidPages;

	TopMostMask mTopMostMask;

	HACCEL mAccel = nullptr;

	// 設定情報
	Settings mSettings;
};

HTREEITEM AppSettingDialog::PImpl::AddPage(
	HTREEITEM parent,
 	AppSettingPageIF* page,
	void* param
)
{
	ASSERT(page != nullptr);
	page->SetParam(param);

	CString name = page->GetName();
	HTREEITEM hItem = mTreeCtrl->InsertItem(name, parent);
	if (parent != TVI_ROOT) {
		mTreeCtrl->Expand(parent, TVE_EXPAND);
	}

	CRect& rcPage = mPageRect;
	CWnd::FromHandle(page->GetHwnd())->MoveWindow(rcPage.left, rcPage.top, rcPage.Width(), rcPage.Height());
	mTreeCtrl->SetItemData(hItem, (DWORD_PTR)page);

	mPages.push_back(page);

	return hItem;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



AppSettingDialog::AppSettingDialog(CWnd* parentWnd) : 
	CDialogEx(IDD_SETTING, parentWnd), 
	in(std::make_unique<PImpl>())
{
	ACCEL accels[1];
	accels[0].cmd = ID_VIEW_HELP;
	accels[0].fVirt = FVIRTKEY;
	accels[0].key = 0x70;   // F1
	in->mAccel = CreateAcceleratorTable(accels, 1);
}

AppSettingDialog::~AppSettingDialog()
{
	for (auto& page : in->mPages) {
		page->Release();
	}

	if (in->mAccel) {
		DestroyAcceleratorTable(in->mAccel);
	}
}

CString AppSettingDialog::GetBreadCrumbsString()
{
	return in->mBreadCrumbs;
}

void AppSettingDialog::SetBreadCrumbsString(const CString& crumbs)
{
	in->mBreadCrumbs = crumbs;
}


void AppSettingDialog::SetSettings(const Settings& settings)
{
	std::unique_ptr<Settings> tmp(settings.Clone());
	in->mSettings.Swap(*tmp.get());
}

const Settings& AppSettingDialog::GetSettings()
{
	return in->mSettings;
}


void AppSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_BREADCRUMBS, in->mBreadCrumbs);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AppSettingDialog, CDialogEx)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_PAGES, OnTvnSelChangingPage)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_APP+1, OnUserEnableOKButton)
	ON_MESSAGE(WM_APP+2, OnUserDisableOKButton)
	ON_WM_NCLBUTTONDOWN()
	ON_COMMAND(ID_VIEW_HELP, OnCommandHelp)
END_MESSAGE_MAP()

#pragma warning( pop )


BOOL AppSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	ModifyStyleEx(0, WS_EX_CONTEXTHELP);

	// パンくずリストの背景色
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

void AppSettingDialog::OnOK()
{
	for (auto& page : in->mPages) {
		if (page->OnKillActive() == false) {
			return;
		}
	}

	// 各プロパティページのOnOKをよぶ
	for (auto& page : in->mPages) {
		page->OnOKCall();
	}
	__super::OnOK();
}

BOOL AppSettingDialog::PreTranslateMessage(MSG* pMsg)
{
	if (in->mAccel && TranslateAccelerator(GetSafeHwnd(), in->mAccel, pMsg)) {
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}

bool AppSettingDialog::SelectPage(HTREEITEM hTreeItem)
{
	if (hTreeItem == in->mLastTreeItem) {
		return true;
	}

	CTreeCtrl* tree = in->mTreeCtrl;
	ASSERT(tree);

	auto newPagePtr = (AppSettingPageIF*)tree->GetItemData(hTreeItem);
	if (newPagePtr == nullptr) {
		return false;
	}

	// 古いページを選択解除する
	if (in->mLastTreeItem) {
		auto oldPagePtr = (AppSettingPageIF*)tree->GetItemData(in->mLastTreeItem);
		if (oldPagePtr) {
			if (oldPagePtr->OnKillActive() == false) {
				return false;
			}
			::ShowWindow(oldPagePtr->GetHwnd(), SW_HIDE);
		}
	}

	// 新しいページを選択する
	newPagePtr->OnSetActive();
	CWnd* tmp = CWnd::FromHandle(newPagePtr->GetHwnd());
	tmp->ShowWindow(SW_SHOW);
	tmp->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	tree->SelectItem(hTreeItem);
	in->mLastTreeItem = hTreeItem;

	// パンくずリスト更新
	BreadCrumbs crumbs(tree, hTreeItem);
	in->mBreadCrumbs = crumbs.ToString();

	UpdateData(FALSE);

	return true;
}

bool AppSettingDialog::ShowHelp()
{
	CTreeCtrl* tree = in->mTreeCtrl;
	ASSERT(tree);

	AppSettingPageIF* newPagePtr = (AppSettingPageIF*)tree->GetItemData(in->mLastTreeItem);
	if (newPagePtr == nullptr) {
		return false;
	}

	CString helpPageId;
	if (newPagePtr->GetHelpPageId(helpPageId) == false) {
		return false;
	}

	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(helpPageId);
	return true;
}

/**
 *  ツリー要素の選択項目変更通知
 */
void AppSettingDialog::OnTvnSelChangingPage(NMHDR *pNMHDR, LRESULT *pResult)
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

HBRUSH AppSettingDialog::OnCtlColor(
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

/**
 	OKボタンを有効化する
 	@return 0
 	@param[in] wp 0
 	@param[in] lp 通知元のページのポインタ
*/
LRESULT AppSettingDialog::OnUserEnableOKButton(
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
LRESULT AppSettingDialog::OnUserDisableOKButton(
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

void AppSettingDialog::OnNcLButtonDown(UINT nHitTest, CPoint pt)
{
	if (nHitTest == HTHELP) {
		ShowHelp();
	}
	else {
		__super::OnNcLButtonDown(nHitTest, pt);
	}
}

void AppSettingDialog::OnCommandHelp()
{
	ShowHelp();
}


//////////////////////


static int CountDepth(const CString& str)
{
	int depth = 0;

	int n = 0;
	CString tok = str.Tokenize(_T("\\"), n);
	while(tok.IsEmpty() == FALSE) {
		depth++;
		tok = str.Tokenize(_T("\\"), n);
	}
	return depth;
}

static void SplitPagePath(const CString& str, CString& parentPath, CString& pageName)
{
	int pos = str.ReverseFind(_T('\\'));
	if (pos == -1) {
		parentPath = _T("");
		pageName = str;
		return;
	}
	parentPath = str.Left(pos);
	pageName = str.Mid(pos + 1);
}

HTREEITEM AppSettingDialog::OnSetupPages()
{
	void* param = &(in->mSettings);

	// すべての設定ページを取得する
	auto repos = launcherapp::settingwindow::AppSettingPageRepository::GetInstance();
	std::vector<AppSettingPageIF*> pages;
	repos->EnumSettingPages(pages);

	std::multimap<std::pair<int, int>, AppSettingPageIF*> orderMap;

	for (auto page : pages) {
		// ページパスを取得
		int order = page->GetOrder();
		CString pagePath = page->GetPagePath();
		pagePath.Trim();
		if (pagePath.IsEmpty()) {
			continue;
		}
		int depth = CountDepth(pagePath);
		orderMap.insert(std::make_pair(std::pair<int,int>(depth, order), page));
	}

	HTREEITEM hFirstItem = nullptr;

	// 親ページから順にHTREEITEMを作成し、HTREEとページ階層を紐づける
	std::map<CString, HTREEITEM> pageTreeMap;
	for (auto item : orderMap) {

		auto page = item.second;

		auto pagePath = page->GetPagePath();

	// ページウインドウを作成する
		if (page->Create(GetSafeHwnd()) == false) {
			page->Release();
			continue;
		}

		// 親階層とページ名に分割
		CString parentPath;
		CString pageName;
		SplitPagePath(pagePath, parentPath, pageName);

		// 親ページとなるHTREEITEMを決定する
		bool isParentRegistered = false;
		HTREEITEM hParent = TVI_ROOT;
		auto itFind2 = pageTreeMap.find(parentPath);
		if (itFind2 != pageTreeMap.end()) {
			hParent = itFind2->second;
			isParentRegistered = true;
		}

		// ページを登録
		auto hTreeItem = in->AddPage(hParent, page, param);
		if (hFirstItem == nullptr) {
			hFirstItem = hTreeItem;
		}

		// HTREEITEMを登録
		if (isParentRegistered == false) {
			pageTreeMap[pagePath] = hTreeItem;
		}
	}
	return hFirstItem;
}


