#include "pch.h"
#include "SnippetGroupEditDialog.h"
#include "commands/snippetgroup/SnippetGroupItemDialog.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/CommandEditValidation.h"
#include "utility/Accessibility.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Command =  launcherapp::core::Command;

namespace launcherapp {
namespace commands {
namespace snippetgroup {



SettingDialog::SettingDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_SNIPPETGROUP, parentWnd),
	mCommandListPtr(nullptr)
{
	SetHelpPageId(_T("SnippetGroupEdit"));
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetName(const CString& name)
{
	mParam.mName = name;
}

void SettingDialog::SetOriginalName(const CString& name)
{
	mOrgName = name;
}

void SettingDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

const SettingDialog::CommandParam& SettingDialog::GetParam() const
{
	return mParam;
}

void SettingDialog::ResetHotKey()
{
	mParam.mHotKeyAttr.Reset();
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_HOTKEY2, mHotKey);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdate)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(IDC_BUTTON_UP, OnButtonUp)
	ON_COMMAND(IDC_BUTTON_DOWN, OnButtonDown)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ITEMS, OnNotifyItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ITEMS, OnNotifyItemDblClk)

END_MESSAGE_MAP()

#pragma warning( pop )

BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mCommandListPtr = (CListCtrl*)GetDlgItem(IDC_LIST_ITEMS);
	ASSERT(mCommandListPtr);

	mCommandListPtr->SetExtendedStyle(mCommandListPtr->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	CString caption;
	GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)mOrgName);

	caption += suffix;
	SetWindowText(caption);

	// ヘッダー追加
	ASSERT(mCommandListPtr);

	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	CString strHeader;
	strHeader.LoadString(IDS_NAME);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 120;
	lvc.fmt = LVCFMT_LEFT;
	mCommandListPtr->InsertColumn(0,&lvc);

	strHeader = _T("説明");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 200;
	lvc.fmt = LVCFMT_LEFT;
	mCommandListPtr->InsertColumn(1,&lvc);

	// 項目を登録
	int index = 0;
	for (auto& item : mParam.mItems) {
		SetItemToList(index++, item);
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool SettingDialog::UpdateStatus()
{
	mMessage.Empty();

	// 128を上限とする
	GetDlgItem(IDC_BUTTON_ADD)->EnableWindow(mParam.mItems.size() < 128);

	mHotKey = mParam.mHotKeyAttr.ToString();
	if (mHotKey.IsEmpty()) {
		mHotKey.LoadString(IDS_NOHOTKEY);
	}

	// ボタンの状態を更新
	POSITION pos = mCommandListPtr->GetFirstSelectedItemPosition();
	bool hasSelect = pos != NULL;

	GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(hasSelect);
	if (hasSelect) {
		int itemIndex = mCommandListPtr->GetNextSelectedItem(pos);
		GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(itemIndex > 0);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(itemIndex < mCommandListPtr->GetItemCount()-1);
	}
	else {
		GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(FALSE);
	}
	// 名前チェック
	bool canPressOK =
	 	launcherapp::commands::common::IsValidCommandName(mParam.mName, mOrgName, mMessage);

	if (canPressOK && mParam.mItems.empty()) {
		mMessage = _T("定型文を登録してください");
		canPressOK = FALSE;
	}

	GetDlgItem(IDOK)->EnableWindow(canPressOK ? TRUE : FALSE);
	return true;
}

void SettingDialog::SwapItem(int srcIndex, int dstIndex)
{
	for (int col = 0; col < 2; ++col) {
		CString srcText = mCommandListPtr->GetItemText(srcIndex, col);
		CString dstText = mCommandListPtr->GetItemText(dstIndex, col);

		mCommandListPtr->SetItemText(srcIndex, col, dstText);
		mCommandListPtr->SetItemText(dstIndex, col, srcText);
	}

	// 選択
	mCommandListPtr->SetItemState(srcIndex, 0, LVIS_SELECTED);
	mCommandListPtr->SetItemState(dstIndex, LVIS_SELECTED, LVIS_SELECTED);

	std::swap(mParam.mItems[srcIndex],mParam.mItems[dstIndex]);
}

void SettingDialog::SetItemToList(int index, const Item& item)
{
	ASSERT(mCommandListPtr);
	if (index < 0) {
		return ;
	}

	if (index == mCommandListPtr->GetItemCount()) {
		mCommandListPtr->InsertItem(index, item.mName);
	}
	else {
		mCommandListPtr->SetItemText(index, 0, item.mName);
	}
	mCommandListPtr->SetItemText(index, 1, item.mDescription);
}

bool SettingDialog::EditItem(int index)
{
	if (index < 0 || mParam.mItems.size() <= index) {
		return false;
	}

	SnippetGroupItemDialog dlg(this);
	dlg.SetItem(mParam.mItems[index]);
	dlg.SetExistingNames(mParam.mItems);
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	auto& item = dlg.GetItem();
	mParam.mItems[index] = item;

	mCommandListPtr->SetItemText(index, 0, item.mName);
	mCommandListPtr->SetItemText(index, 1, item.mDescription);


	return true;
}


void SettingDialog::OnUpdate()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH SettingDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (::utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

void SettingDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	__super::OnOK();
}


void SettingDialog::OnButtonHotKey()
{
	UpdateData();

	if (CommandHotKeyDialog::ShowDialog(mParam.mName, mParam.mHotKeyAttr, this) == false) {
		return ;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonAdd()
{
	UpdateData();

	ASSERT(mCommandListPtr);

	SnippetGroupItemDialog dlg(this);
	dlg.SetExistingNames(mParam.mItems);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	const auto& newItem = dlg.GetItem();

	int nItemCount = mCommandListPtr->GetItemCount();
	int index = mCommandListPtr->InsertItem(nItemCount, newItem.mName);
	mCommandListPtr->SetItemText(index, 1, newItem.mDescription);
	mParam.mItems.push_back(newItem);

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonEdit()
{
	UpdateData();

	POSITION pos = mCommandListPtr->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = mCommandListPtr->GetNextSelectedItem(pos);
	EditItem(itemIndex);

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonDelete()
{
	POSITION pos = mCommandListPtr->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = mCommandListPtr->GetNextSelectedItem(pos);
	mCommandListPtr->DeleteItem(itemIndex);
	mParam.mItems.erase(mParam.mItems.begin() + itemIndex);

	if (itemIndex < mCommandListPtr->GetItemCount()) {
		mCommandListPtr->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
	}
	else {
		itemIndex--;
		if (0 <= itemIndex && itemIndex < mCommandListPtr->GetItemCount()) {
			mCommandListPtr->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonUp()
{
	POSITION pos = mCommandListPtr->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = mCommandListPtr->GetNextSelectedItem(pos);
	if (itemIndex == 0) {
		return;
	}
	SwapItem(itemIndex, itemIndex-1);

	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonDown()
{
	POSITION pos = mCommandListPtr->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = mCommandListPtr->GetNextSelectedItem(pos);
	if (itemIndex == mCommandListPtr->GetItemCount()-1) {
		return;
	}
	SwapItem(itemIndex, itemIndex+1);

	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
	*pResult = 0;
}

void SettingDialog::OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult)
{
	ASSERT(mCommandListPtr);

	*pResult = 0;

	NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;

	int index = nm->iItem;
	if (index < 0 || mParam.mItems.size() <= index) {
		return;
	}

	UpdateData();
	EditItem(index);
	UpdateStatus();
	UpdateData(FALSE);
}

} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

