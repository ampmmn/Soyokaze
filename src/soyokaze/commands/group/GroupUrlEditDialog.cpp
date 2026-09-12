#include "pch.h"
#include "GroupUrlEditDialog.h"
#include "resource.h"
#include "utility/Accessibility.h"
#include "utility/Regex.h"

namespace launcherapp { namespace commands { namespace group {

namespace {

struct SITE_ITEM
{
	int mID;
	LPCTSTR mSiteName;
	LPCTSTR mURL;
};

const std::vector<SITE_ITEM> SITE_TEMPLATE = {
	{ 1, _T("&Google"), _T("https://www.google.com/search?q=$*") },
	{ 2, _T("&Bing"), _T("https://www.bing.com/search?q=$*") },
	{ 3, _T("&DuckDuckGo"), _T("https://duckduckgo.com/?t=h_&q=$*") },
	{ 4, _T("&X"), _T("https://x.com/search?q=$*") },
	{ 5, _T("&Amazon"), _T("https://www.amazon.co.jp/s?k=$*") },
	{ 6, _T("&Youtube"), _T("https://www.youtube.com/results?search_query=$*") },
};

}

GroupUrlEditDialog::GroupUrlEditDialog(CWnd* parentWnd) : 
	launcherapp::control::SinglePageDialog(IDD_GROUP_URL, parentWnd)
{
	mItem.mType = GroupItemType::URL;
	SetHelpPageId("GroupEdit");
}

void GroupUrlEditDialog::SetItem(const GroupItem& item)
{
	mItem = item;
	mItem.mType = GroupItemType::URL;
}

const GroupItem& GroupUrlEditDialog::GetItem() const
{
	return mItem;
}

void GroupUrlEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_GROUP_URL, mItem.mItemName);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Control(pDX, IDC_BUTTON_MENU, mSiteMenuButton);
}

BOOL GroupUrlEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mSiteMenu.CreatePopupMenu();
	for (const auto& item : SITE_TEMPLATE) {
		mSiteMenu.InsertMenu((UINT)-1, 0, item.mID, item.mSiteName);
	}
	mSiteMenuButton.m_hMenu = (HMENU)mSiteMenu;

	GetDlgItem(IDC_EDIT_GROUP_URL)->SendMessage(EM_SETCUEBANNER, TRUE,
		(LPARAM)(LPCTSTR)_T("右にある▼ボタンからプリセットのURLを入力できます"));

	UpdateData(FALSE);
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void GroupUrlEditDialog::OnOK()
{
	UpdateData(TRUE);
	if (UpdateStatus() == false) {
		return;
	}
	__super::OnOK();
}

BEGIN_MESSAGE_MAP(GroupUrlEditDialog, launcherapp::control::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_GROUP_URL, OnUpdateStatus)
	ON_BN_CLICKED(IDC_BUTTON_MENU, OnSiteMenuButtonClicked)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/**
  URL欄の入力内容を検証してOKボタンの状態を更新する
  @return true:有効  false:無効
*/
bool GroupUrlEditDialog::UpdateStatus()
{
	static const launcherapp::utility::Regex regHttp(_T("^https?://.+$"));
	bool isValid = false;
	if (mItem.mItemName.IsEmpty()) {
		mMessage = _T("URLを入力してください");
	}
	else if (regHttp.FullMatch(mItem.mItemName) == false) {
		mMessage = _T("URLは http:// か https:// で始まる必要があります");
	}
	else {
		isValid = true;
		mMessage.Empty();
	}
	GetDlgItem(IDOK)->EnableWindow(isValid ? TRUE : FALSE);
	return isValid;
}

/**
  URL欄の入力内容が変更されたときにOKボタンの状態を更新する
*/
void GroupUrlEditDialog::OnUpdateStatus()
{
	UpdateData(TRUE);
	UpdateStatus();
	UpdateData(FALSE);
}

/**
  ステータスメッセージの表示色を設定する
*/
HBRUSH GroupUrlEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (::utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mMessage.IsEmpty() ? RGB(0, 0, 0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

/**
  よく使う検索URLの選択結果をURL欄へ反映する
*/
void GroupUrlEditDialog::OnSiteMenuButtonClicked()
{
	UpdateData(TRUE);

	int id = mSiteMenuButton.m_nMenuResult;
	for (const auto& item : SITE_TEMPLATE) {
		if (id != item.mID) {
			continue;
		}

		mItem.mItemName = item.mURL;
		UpdateData(FALSE);
		UpdateStatus();
		UpdateData(FALSE);
		break;
	}
}

}}}
