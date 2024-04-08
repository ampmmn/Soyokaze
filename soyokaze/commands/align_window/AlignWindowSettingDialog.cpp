#include "pch.h"
#include "AlignWindowSettingDialog.h"
#include "commands/align_window/AlignWindowItemSettingDialog.h"
#include "commands/common/Message.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "icon/CaptureIconLabel.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/TopMostMask.h"
#include "utility/ProcessPath.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "commands/core/CommandRepository.h"
#include "resource.h"

using namespace soyokaze::commands::common;

namespace soyokaze {
namespace commands {
namespace align_window {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct SettingDialog::PImpl
{
	// 設定情報
	CommandParam mParam;
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

	// ウインドウ一覧リスト
	CListCtrl* mListPtr = nullptr;

	// ホットキー(表示用)
	CString mHotKey;

	// ウインドウキャプチャ用アイコン
	//CaptureIconLabel mIconLabel;

	TopMostMask mTopMostMask;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


SettingDialog::SettingDialog() : 
	CDialogEx(IDD_ALIGNWINDOWEDIT),
	in(std::make_unique<PImpl>())
{
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetParam(const Param& param)
{
	in->mParam = param;
}

const SettingDialog::Param&
SettingDialog::GetParam()
{
	return in->mParam;
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, in->mHotKey);
	DDX_Check(pDX, IDC_CHECK_NOTIFYIFNOTEXIST, in->mParam.mIsNotifyIfWindowNotFound);
	DDX_Check(pDX, IDC_CHECK_KEEPACTIVEWINDOW, in->mParam.mIsKeepActiveWindow);
}

BEGIN_MESSAGE_MAP(SettingDialog, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_COMMAND(IDC_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(IDC_BUTTON_UP, OnButtonUp)
	ON_COMMAND(IDC_BUTTON_DOWN, OnButtonDown)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_COMMANDS, OnNotifyItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_COMMANDS, OnNotifyItemDblClk)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mListPtr = (CListCtrl*)GetDlgItem(IDC_LIST_COMMANDS);
	CListCtrl* listWndPtr = in->mListPtr;
	ASSERT(listWndPtr);
	listWndPtr->SetExtendedStyle(in->mListPtr->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	// ヘッダー追加
	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	CString strHeader;
	strHeader.LoadString(IDS_WINDOWTITLE);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 110;
	lvc.fmt = LVCFMT_LEFT;
	listWndPtr->InsertColumn(0,&lvc);

	strHeader.LoadString(IDS_WINDOWCLASS);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 110;
	lvc.fmt = LVCFMT_LEFT;
	listWndPtr->InsertColumn(1,&lvc);

	strHeader.LoadString(IDS_X);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 60;
	lvc.fmt = LVCFMT_LEFT;
	listWndPtr->InsertColumn(2,&lvc);

	strHeader.LoadString(IDS_Y);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 60;
	lvc.fmt = LVCFMT_LEFT;
	listWndPtr->InsertColumn(3,&lvc);

	strHeader.LoadString(IDS_WIDTH);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 50;
	lvc.fmt = LVCFMT_LEFT;
	listWndPtr->InsertColumn(4,&lvc);

	strHeader.LoadString(IDS_HEIGHT);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 50;
	lvc.fmt = LVCFMT_LEFT;
	listWndPtr->InsertColumn(5,&lvc);

	// 項目を登録
	int index = 0;
	for (auto& item : in->mParam.mItems) {
		SetItemToList(index++, item);
	}

	in->mOrgName = in->mParam.mName;

	CString caption;
	GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	UpdateStatus();

	UpdateData(FALSE);

	return TRUE;
}

void SettingDialog::OnButtonHotKey()
{
	UpdateData();

	CommandHotKeyDialog dlg(in->mParam.mHotKeyAttr, in->mParam.mIsGlobal);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(in->mParam.mHotKeyAttr);
	in->mParam.mIsGlobal = dlg.IsGlobal();
	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	UpdateData(FALSE);
}

void SettingDialog::OnOK()
{
	UpdateData();
	__super::OnOK();
}

bool SettingDialog::UpdateStatus()
{
	// ボタンの状態を更新
	POSITION pos = in->mListPtr->GetFirstSelectedItemPosition();
	bool hasSelect = pos != NULL;
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(hasSelect);
	GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(hasSelect);
	if (hasSelect) {
		int itemIndex = in->mListPtr->GetNextSelectedItem(pos);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(itemIndex > 0);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(itemIndex < in->mListPtr->GetItemCount()-1);
	}
	else {
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(FALSE);
	}

	const CString& name = in->mParam.mName;
	if (name.IsEmpty()) {
		in->mMessage = _T("コマンド名を入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	if (in->mParam.mItems.size() == 0) {
		in->mMessage = _T("整列するウインドウを追加してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	in->mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();

	// 重複チェック
	if (name.CompareNoCase(in->mOrgName) != 0) {
		auto cmd = cmdRepoPtr->QueryAsWholeMatch(name, false);
		if (cmd != nullptr) {
			cmd->Release();
			in->mMessage.LoadString(IDS_ERR_NAMEALREADYEXISTS);
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			return false;
		}
	}
	// 使えない文字チェック
	if (cmdRepoPtr->IsValidAsName(name) == false) {
		in->mMessage.LoadString(IDS_ERR_ILLEGALCHARCONTAINS);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	return true;
}

void SettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonAdd()
{
	ItemDialog dlg(this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	auto& item = dlg.GetParam();

	int itemIndex = (int)in->mParam.mItems.size();
	in->mParam.mItems.push_back(item);
	SetItemToList(itemIndex, item);

	UpdateData(FALSE);
}

void SettingDialog::OnButtonEdit()
{
	CListCtrl* listWndPtr = in->mListPtr;

	POSITION pos = listWndPtr->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		// リスト選択項目がなければなにもしない
		return;
	}

	// リスト項目と対応するItemを削除
	int itemIndex = listWndPtr->GetNextSelectedItem(pos);
	auto& item = in->mParam.mItems[itemIndex];

	ItemDialog dlg(this);
	dlg.SetParam(item);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	item = dlg.GetParam();
	SetItemToList(itemIndex, item);

	UpdateStatus();
}

void SettingDialog::OnButtonDelete()
{
	CListCtrl* listWndPtr = in->mListPtr;

	POSITION pos = listWndPtr->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		// リスト選択項目がなければなにもしない
		return;
	}

	// リスト項目と対応するItemを削除
	int itemIndex = listWndPtr->GetNextSelectedItem(pos);
	listWndPtr->DeleteItem(itemIndex);
	in->mParam.mItems.erase(in->mParam.mItems.begin() + itemIndex);

	// 削除した後のリスト上の選択項目の調整
	if (itemIndex < listWndPtr->GetItemCount()) {
		// 削除後に詰めた要素を選択する
		listWndPtr->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
	}
	else {
		// 末尾を削除した場合は、選択位置をずらす
		itemIndex--;
		if (0 <= itemIndex && itemIndex < listWndPtr->GetItemCount()) {
			listWndPtr->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonUp()
{
	CListCtrl* listWndPtr = in->mListPtr;

	POSITION pos = listWndPtr->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listWndPtr->GetNextSelectedItem(pos);
	if (itemIndex == 0) {
		return;
	}
	SwapItem(itemIndex, itemIndex-1);

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonDown()
{
	CListCtrl* listWndPtr = in->mListPtr;

	POSITION pos = listWndPtr->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listWndPtr->GetNextSelectedItem(pos);
	if (itemIndex == listWndPtr->GetItemCount()-1) {
		return;
	}
	SwapItem(itemIndex, itemIndex+1);

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateStatus();
	UpdateData(FALSE);
	*pResult = 0;
}

void SettingDialog::OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult)
{
	ASSERT(in->mListPtr);

	*pResult = 0;

	NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;

	int index = nm->iItem;
	if (index < 0 || in->mParam.mItems.size() <= index) {
		return;
	}

	auto& item = in->mParam.mItems[index];

	ItemDialog dlg(this);
	dlg.SetParam(item);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	item = dlg.GetParam();
	SetItemToList(index, item);

	UpdateStatus();
}

void SettingDialog::SwapItem(int srcIndex, int dstIndex)
{
	CListCtrl* listWndPtr = in->mListPtr;

	for (int col = 0; col < 6; ++col) {
		CString srcText = listWndPtr->GetItemText(srcIndex, col);
		CString dstText = listWndPtr->GetItemText(dstIndex, col);

		listWndPtr->SetItemText(srcIndex, col, dstText);
		listWndPtr->SetItemText(dstIndex, col, srcText);
	}

	// 選択
	listWndPtr->SetItemState(dstIndex, 0, LVIS_SELECTED);
	listWndPtr->SetItemState(dstIndex, LVIS_SELECTED, LVIS_SELECTED);

	std::swap(in->mParam.mItems[srcIndex], in->mParam.mItems[dstIndex]);
}

void SettingDialog::SetItemToList(int index, const Param::ITEM& item)
{
	CListCtrl* listWndPtr = in->mListPtr;
	ASSERT(listWndPtr);
	if (index < 0) {
		return ;
	}

  CString text;
	if (index == listWndPtr->GetItemCount()) {
		listWndPtr->InsertItem(index, _T(""));
	}
	listWndPtr->SetItemText(index, 0, item.mCaptionStr);
	listWndPtr->SetItemText(index, 1, item.mClassStr);

	const WINDOWPLACEMENT& wp = item.mPlacement;

	if (item.mAction == Param::AT_SETPOS) {
		text.Format(_T("%d"), wp.rcNormalPosition.left);
		listWndPtr->SetItemText(index, 2, text);
		text.Format(_T("%d"), wp.rcNormalPosition.top);
		listWndPtr->SetItemText(index, 3, text);
		text.Format(_T("%d"), wp.rcNormalPosition.right - wp.rcNormalPosition.left);
		listWndPtr->SetItemText(index, 4, text);
		text.Format(_T("%d"), wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
		listWndPtr->SetItemText(index, 5, text);
	}
	else {
		if (item.mAction == Param::AT_MAXIMIZE) {
			listWndPtr->SetItemText(index, 2, _T("(最大化)"));
		}
		else if (item.mAction == Param::AT_MINIMIZE){
			listWndPtr->SetItemText(index, 2, _T("(最小化)"));
		}
		else if (item.mAction == Param::AT_HIDE){
			listWndPtr->SetItemText(index, 2, _T("(非表示)"));
		}

		listWndPtr->SetItemText(index, 3, _T(""));
		listWndPtr->SetItemText(index, 4, _T(""));
		listWndPtr->SetItemText(index, 5, _T(""));
	}
}

HBRUSH SettingDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = in->mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

}
}
}
