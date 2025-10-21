#include "pch.h"
#include "KeySplitterEditDialog.h"
#include "commands/keysplitter/KeySplitterModifierDialog.h"
#include "commands/validation/CommandEditValidation.h"
#include "utility/Accessibility.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace keysplitter {

SettingDialog::SettingDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_KEYSPLITTER, parentWnd),
	mCommandListPtr(nullptr)
{
	SetHelpPageId("KeySplitterEdit");
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

const CommandParam& SettingDialog::GetParam() const
{
	return mParam;
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mParam.mDescription);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdate)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_COMMANDS, OnNotifyItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_COMMANDS, OnNotifyItemDblClk)

END_MESSAGE_MAP()

#pragma warning( pop )

BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mCommandListPtr = (CListCtrl*)GetDlgItem(IDC_LIST_COMMANDS);
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
	strHeader = _T("キー");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 50;
	lvc.fmt = LVCFMT_LEFT;
	mCommandListPtr->InsertColumn(0,&lvc);

	strHeader = _T("コマンド");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 140;
	lvc.fmt = LVCFMT_LEFT;
	mCommandListPtr->InsertColumn(1,&lvc);

	strHeader = _T("ガイド");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 140;
	lvc.fmt = LVCFMT_LEFT;
	mCommandListPtr->InsertColumn(2,&lvc);

	ITEM item;

	// 項目を登録
	int index = 0;
	for (int i = 0; i < 16; ++i) {

		ModifierState state(i);
		if (mParam.GetMapping(state, item) == false) {
			continue;
		}

		SetItemToList(index++, state, item);
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool SettingDialog::UpdateStatus()
{
	mMessage.Empty();

	// ボタンの状態を更新
	POSITION pos = mCommandListPtr->GetFirstSelectedItemPosition();
	bool hasSelect = pos != NULL;

	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(hasSelect);
	GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(hasSelect);

	// 名前チェック
	bool canPressOK =
	 	launcherapp::commands::validation::IsValidCommandName(mParam.mName, mOrgName, mMessage);

	ITEM item;
	ModifierState stateEmpty;
	if (canPressOK && mParam.GetMapping(stateEmpty, item) == false) {
		mMessage = _T("Enterキーに対する割り当てを設定してください");
		canPressOK = FALSE;
	}

	GetDlgItem(IDOK)->EnableWindow(canPressOK ? TRUE : FALSE);
	return true;
}

void SettingDialog::SetItemToList(
	int index,
	const ModifierState& state,
	const ITEM& item
)
{
	ASSERT(mCommandListPtr);
	if (index < 0) {
		return ;
	}

	if (index == mCommandListPtr->GetItemCount()) {
		mCommandListPtr->InsertItem(index, state.ToString());
		mCommandListPtr->SetItemData(index, state.ToBits());
	}
	else {
		mCommandListPtr->SetItemText(index, 0, state.ToString());
	}

	mCommandListPtr->SetItemText(index, 1, item.mCommandName);
	mCommandListPtr->SetItemText(index, 2, item.mActionName);
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

bool SettingDialog::EditItem(int index)
{
	ModifierState state((int)mCommandListPtr->GetItemData(index));

	ITEM item;
	mParam.GetMapping(state, item);

	ModifierDialog dlg(this);
	dlg.SetModifierState(state);
	dlg.SetItem(item);
	dlg.SetParam(mParam);
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// 更新後の状態を取得
	dlg.GetItem(item);
	ModifierState newState;
	dlg.GetModifierState(newState);

	if (state != newState) {
		mParam.EraseMapping(state);
	}
	mParam.SetMapping(newState, item);

	SetItemToList(index, newState, item);
	return true;
}


void SettingDialog::OnButtonAdd()
{
	ASSERT(mCommandListPtr);
	UpdateData();

	ModifierDialog dlg(this);
	dlg.SetParam(mParam);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	ModifierState state;
	dlg.GetModifierState(state);
	ITEM item;
	dlg.GetItem(item);

	mParam.SetMapping(state, item);

	// 項目を追加する
	int nItemCount = mCommandListPtr->GetItemCount();

	CRect rc;
	mCommandListPtr->GetItemRect(0, &rc, LVIR_BOUNDS);
	int offset = rc.Height() * (nItemCount - mCommandListPtr->GetTopIndex());
	CSize cs;
	cs.cx = 0;
	cs.cy = offset;
	if (offset) {
		mCommandListPtr->Scroll(cs);
	}

	SetItemToList(nItemCount, state, item);

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonEdit()
{
	UpdateData();

	POSITION pos = mCommandListPtr->GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return;
	}

	int itemIndex = mCommandListPtr->GetNextSelectedItem(pos);
	if (EditItem(itemIndex) == false) {
		return;
	}

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonDelete()
{
	UpdateData();

	POSITION pos = mCommandListPtr->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = mCommandListPtr->GetNextSelectedItem(pos);

	ModifierState state((int)mCommandListPtr->GetItemData(itemIndex));

	mCommandListPtr->DeleteItem(itemIndex);

	mParam.DeleteMapping(state);

	if (itemIndex < mCommandListPtr->GetItemCount()) {
		mCommandListPtr->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
	}
	else {
		itemIndex--;
		if (0 <= itemIndex && itemIndex < mCommandListPtr->GetItemCount()) {
			mCommandListPtr->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

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
	if (index < 0 || 16 <= index) {
		return;
	}

	if (EditItem(index) == false) {
		return;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

} // end of namespace keysplitter
} // end of namespace commands
} // end of namespace launcherapp

