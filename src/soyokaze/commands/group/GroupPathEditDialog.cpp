#include "pch.h"
#include "GroupPathEditDialog.h"
#include "control/FolderDialog.h"
#include "resource.h"
#include "utility/Accessibility.h"
#include "utility/Path.h"

namespace launcherapp { namespace commands { namespace group {

GroupPathEditDialog::GroupPathEditDialog(CWnd* parentWnd) :
	launcherapp::control::SinglePageDialog(IDD_GROUP_PATH, parentWnd)
{
	mItem.mType = GroupItemType::Path;
	SetHelpPageId("GroupEdit");
}

void GroupPathEditDialog::SetItem(const GroupItem& item)
{
	mItem = item;
	mItem.mType = GroupItemType::Path;
}

const GroupItem& GroupPathEditDialog::GetItem() const
{
	return mItem;
}

void GroupPathEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_GROUP_PATH, mItem.mItemName);
	DDX_Text(pDX, IDC_EDIT_GROUP_PARAM, mItem.mParam);
	DDX_Text(pDX, IDC_EDIT_GROUP_WORKDIR, mItem.mWorkDir);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_CBIndex(pDX, IDC_COMBO_GROUP_SHOWTYPE, mShowIndex);
	DDX_Control(pDX, IDC_BUTTON_MENU, mPathMenuButton);
}

BOOL GroupPathEditDialog::OnInitDialog()
{
	__super::OnInitDialog();
	GetDlgItem(IDC_BUTTON_BROWSEDIR3)->SetWindowTextW(L"\U0001F4C2");

	mPathMenu.CreatePopupMenu();
	mPathMenu.InsertMenu((UINT)-1, 0, 1, _T("ファイル選択"));
	mPathMenu.InsertMenu((UINT)-1, 0, 2, _T("フォルダ選択"));
	mPathMenuButton.m_hMenu = (HMENU)mPathMenu;

	auto combo = (CComboBox*)GetDlgItem(IDC_COMBO_GROUP_SHOWTYPE);
	combo->AddString(_T("通常"));
	combo->AddString(_T("非表示"));
	combo->AddString(_T("最小化"));
	combo->AddString(_T("最大化"));
	mShowIndex = 0;
	if (mItem.mShowType == SW_HIDE) {
		mShowIndex = 1;
	}
	else if (mItem.mShowType == SW_MINIMIZE) {
		mShowIndex = 2;
	}
	else if (mItem.mShowType == SW_MAXIMIZE) {
		mShowIndex = 3;
	}
	UpdateData(FALSE);
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void GroupPathEditDialog::OnOK()
{
	UpdateData(TRUE);
	if (UpdateStatus() == false) {
		return;
	}

	const int showTypes[] = { SW_SHOW, SW_HIDE, SW_MINIMIZE, SW_MAXIMIZE };
	if (0 <= mShowIndex && mShowIndex < 4) {
		mItem.mShowType = showTypes[mShowIndex];
	}
	__super::OnOK();
}

/**
  パスの存在を検証してOKボタンの状態を更新する
  @return true:有効  false:無効
*/
bool GroupPathEditDialog::UpdateStatus()
{
	bool isValid = false;
	if (mItem.mItemName.IsEmpty()) {
		mMessage = _T("パスを入力してください");
	}
	else if (Path::FileExists(mItem.mItemName) == false) {
		mMessage = _T("指定されたパスは存在しません");
	}
	else {
		isValid = true;
		mMessage.Empty();
	}
	GetDlgItem(IDOK)->EnableWindow(isValid ? TRUE : FALSE);
	return isValid;
}

/**
  ステータスメッセージの表示色を設定する
*/
HBRUSH GroupPathEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
  作業フォルダ選択ダイアログを表示して作業フォルダを設定する
*/
void GroupPathEditDialog::OnButtonBrowseDir3Clicked()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mItem.mWorkDir, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mItem.mWorkDir = dlg.GetPathName();
	UpdateData(FALSE);
}

/**
  ファイル選択ダイアログを表示してパスを設定する
*/
void GroupPathEditDialog::OnButtonBrowseFileClicked()
{
	UpdateData();

	CFileDialog dlg(TRUE, nullptr, mItem.mItemName, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mItem.mItemName = dlg.GetPathName();
	UpdateData(FALSE);
	UpdateStatus();
	UpdateData(FALSE);
}

/**
  フォルダ選択ダイアログを表示してパスを設定する
*/
void GroupPathEditDialog::OnButtonBrowseDirClicked()
{
	UpdateData();

	CFolderDialog dlg(_T(""), mItem.mItemName, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mItem.mItemName = dlg.GetPathName();
	UpdateData(FALSE);
	UpdateStatus();
	UpdateData(FALSE);
}

/**
  パス選択メニューの選択結果に応じた処理を実行する
*/
void GroupPathEditDialog::OnPathMenuButtonClicked()
{
	switch (mPathMenuButton.m_nMenuResult) {
		case 1:
			OnButtonBrowseFileClicked();
			break;
		case 2:
			OnButtonBrowseDirClicked();
			break;
	}
}

BEGIN_MESSAGE_MAP(GroupPathEditDialog, launcherapp::control::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_GROUP_PATH, OnUpdateStatus)
	ON_BN_CLICKED(IDC_BUTTON_MENU, OnPathMenuButtonClicked)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR3, OnButtonBrowseDir3Clicked)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/**
  パス欄の入力内容が変更されたときにOKボタンの状態を更新する
*/
void GroupPathEditDialog::OnUpdateStatus()
{
	UpdateData(TRUE);
	UpdateStatus();
	UpdateData(FALSE);
}

}}}
