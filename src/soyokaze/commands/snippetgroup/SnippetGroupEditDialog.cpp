#include "pch.h"
#include "SnippetGroupEditDialog.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/validation/CommandEditValidation.h"
#include "utility/Accessibility.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>
#include <set>

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
	SetHelpPageId("SnippetGroupEdit");


	ACCEL accels[4];
	accels[0].cmd = ID_VIEW_NEXT;
	accels[0].fVirt = FVIRTKEY;
	accels[0].key = 0x73;   // F4
	accels[1].cmd = ID_VIEW_PREV;
	accels[1].fVirt = FVIRTKEY | FSHIFT;  // Shift
	accels[1].key = 0x73;   // F4
	accels[2].cmd = ID_VIEW_UP;
	accels[2].fVirt = FVIRTKEY | FALT;  // ALT
	accels[2].key = 0x26;   // 上
	accels[3].cmd = ID_VIEW_DOWN;
	accels[3].fVirt = FVIRTKEY | FALT;  // ALT
	accels[3].key = 0x28;   // 下

	mAccel = CreateAcceleratorTable(accels, 4);
}

SettingDialog::~SettingDialog()
{
	if (mAccel) {
		DestroyAcceleratorTable(mAccel);
	}
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
	if (mCurItem) {
		DDX_Text(pDX, IDC_EDIT_NAME2, mCurItem->mName);
		DDX_Text(pDX, IDC_EDIT_DESCRIPTION2, mCurItem->mDescription);
		DDX_Text(pDX, IDC_EDIT_TEXT, mCurItem->mText);
	}
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdate)
	ON_EN_CHANGE(IDC_EDIT_NAME2, OnUpdateListItem)
	ON_EN_CHANGE(IDC_EDIT_DESCRIPTION2, OnUpdateListItem)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(IDC_BUTTON_UP, OnButtonUp)
	ON_COMMAND(IDC_BUTTON_DOWN, OnButtonDown)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ITEMS, OnNotifyItemChanged)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_COMMAND(ID_VIEW_PREV, OnViewPrev)
	ON_COMMAND(ID_VIEW_NEXT, OnViewNext)
	ON_COMMAND(ID_VIEW_UP, OnViewUp)
	ON_COMMAND(ID_VIEW_DOWN, OnViewDown)

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

	if (index > 0) {
		mCommandListPtr->SetItemState(0, 0, LVIS_SELECTED | LVIS_FOCUSED);
		mCommandListPtr->SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		mCurItem = &(mParam.mItems[0]);
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

	// ホットキー
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
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(itemIndex > 0);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(itemIndex < mCommandListPtr->GetItemCount()-1);
		GetDlgItem(IDC_EDIT_NAME2)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_DESCRIPTION2)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_TEXT)->EnableWindow(TRUE);

		mCommandListPtr->SetItemText(itemIndex, 0, mCurItem->mName);
		mCommandListPtr->SetItemText(itemIndex, 1, mCurItem->mDescription);
		//mCommandListPtr->SetItemText(itemIndex, 0, mCurItem->mName);

		ASSERT(mCurItem);

	}
	else {
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_NAME2)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DESCRIPTION2)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_TEXT)->EnableWindow(FALSE);
	}
	// 名前チェック
	bool canPressOK =
	 	launcherapp::commands::validation::IsValidCommandName(mParam.mName, mOrgName, mMessage);

	if (canPressOK && mParam.mItems.empty()) {
		mMessage = _T("定型文を登録してください");
		canPressOK = FALSE;
	}

	// 項目名に重複がないかチェック
	std::set<CString> nameSet;
	for (auto& item : mParam.mItems) {

		if (item.mName.IsEmpty()) {
			mMessage = _T("定型文の名前を入力してください");
			canPressOK = FALSE;
			break;
		}
		if (nameSet.find(item.mName) != nameSet.end()) {
			mMessage = _T("名前が重複している定型文があります");
			canPressOK = FALSE;
			break;
		}
		nameSet.insert(item.mName);
	}

	GetDlgItem(IDOK)->EnableWindow(canPressOK ? TRUE : FALSE);
	return true;
}

void SettingDialog::SwapItem(int srcIndex, int dstIndex)
{
	mIsItemChanging = true;
	for (int col = 0; col < 2; ++col) {
		CString srcText = mCommandListPtr->GetItemText(srcIndex, col);
		CString dstText = mCommandListPtr->GetItemText(dstIndex, col);

		mCommandListPtr->SetItemText(srcIndex, col, dstText);
		mCommandListPtr->SetItemText(dstIndex, col, srcText);
	}

	// 選択
	mCommandListPtr->SetItemState(srcIndex, 0, LVIS_SELECTED | LVIS_FOCUSED);
	mCommandListPtr->SetItemState(dstIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	std::swap(mParam.mItems[srcIndex],mParam.mItems[dstIndex]);
	mCurItem = &(mParam.mItems[dstIndex]);

	mIsItemChanging = false;
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

void SettingDialog::OnUpdate()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnUpdateListItem()
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

BOOL SettingDialog::PreTranslateMessage(MSG* pMsg)
{
	if (mAccel && TranslateAccelerator(GetSafeHwnd(), mAccel, pMsg)) {
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
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

	// 新しい項目を追加
	Item newItem;
	newItem.mName.Format(_T("新しい定型文-%d"), mParam.mItems.size()+1);
	newItem.mText = _T("(ここに定型文の内容を入力します)");

	int nItemCount = mCommandListPtr->GetItemCount();
	int index = mCommandListPtr->InsertItem(nItemCount, newItem.mName);
	mCommandListPtr->SetItemText(index, 1, newItem.mDescription);
	mParam.mItems.push_back(newItem);

	mCurItem = &(mParam.mItems[mParam.mItems.size()-1]);
	UpdateData(FALSE);

	// 新しい項目をリスト上で選択状態にする
	mCommandListPtr->SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	auto nameEdit = (CEdit*)GetDlgItem(IDC_EDIT_NAME2);
	// キャレット全選択
	nameEdit->SetSel((DWORD)0xFFFF0000, FALSE);


	UpdateStatus();
	UpdateData(FALSE);

	// 新しい項目名にフォーカスを設定する
	GetDlgItem(IDC_EDIT_NAME2)->SetFocus();
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
	*pResult = 0;

	if (mIsItemChanging) {
		return ;
	}

	auto nmLV = (NMLISTVIEW*)pNMHDR;

	Item* newSelectedItem = nullptr;
	
	int index = nmLV->iItem;
	if (index != -1 && index < mParam.mItems.size()) {
		newSelectedItem = &(mParam.mItems[index]);
	}
	else {
		newSelectedItem = nullptr;
	}

	if (newSelectedItem == mCurItem) {
		// 選択状態に変更なし
		return;
	}

	UpdateData();

	mCurItem = newSelectedItem;

	UpdateStatus();
	UpdateData(FALSE);
}

// マニュアル表示
void SettingDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate("MacroList");
	*pResult = 0;
}


void SettingDialog::OnViewPrev()
{
	POSITION pos = mCommandListPtr->GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return ;
	}

	int itemIndex = mCommandListPtr->GetNextSelectedItem(pos);

	int newItemIndex = itemIndex-1;
	if (newItemIndex < 0) {
		newItemIndex = mCommandListPtr->GetItemCount()-1;
	}
	if (itemIndex == newItemIndex) {
		return ;
	}

	// 選択
	mCommandListPtr->SetItemState(itemIndex, 0, LVIS_SELECTED | LVIS_FOCUSED);
	mCommandListPtr->SetItemState(newItemIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	mCurItem = &(mParam.mItems[newItemIndex]);

	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnViewNext()
{
	POSITION pos = mCommandListPtr->GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return ;
	}

	int itemIndex = mCommandListPtr->GetNextSelectedItem(pos);

	int newItemIndex = itemIndex+1;
	if (newItemIndex >= mCommandListPtr->GetItemCount()) {
		newItemIndex = 0;
	}
	if (itemIndex == newItemIndex) {
		return ;
	}

	// 選択
	mCommandListPtr->SetItemState(itemIndex, 0, LVIS_SELECTED | LVIS_FOCUSED);
	mCommandListPtr->SetItemState(newItemIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	mCurItem = &(mParam.mItems[newItemIndex]);

	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnViewUp()
{
	if (GetFocus() == GetDlgItem(IDC_EDIT_NAME2)) {
		GetDlgItem(IDC_EDIT_TEXT)->SetFocus();
	}
	else if (GetFocus() == GetDlgItem(IDC_EDIT_DESCRIPTION2)) {
		GetDlgItem(IDC_EDIT_NAME2)->SetFocus();
	}
	else if (GetFocus() == GetDlgItem(IDC_EDIT_TEXT)) {
		GetDlgItem(IDC_EDIT_DESCRIPTION2)->SetFocus();
	}
}

void SettingDialog::OnViewDown()
{
	if (GetFocus() == GetDlgItem(IDC_EDIT_NAME2)) {
		GetDlgItem(IDC_EDIT_DESCRIPTION2)->SetFocus();
	}
	else if (GetFocus() == GetDlgItem(IDC_EDIT_DESCRIPTION2)) {
		GetDlgItem(IDC_EDIT_TEXT)->SetFocus();
	}
	else if (GetFocus() == GetDlgItem(IDC_EDIT_TEXT)) {
		GetDlgItem(IDC_EDIT_NAME2)->SetFocus();
	}
}

} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

