#include "pch.h"
#include "framework.h"
#include "ShellExecEditEnvPage.h"
#include "commands/shellexecute/ShellExecCommandParam.h"
#include "commands/shellexecute/ShellExecEnvValueEditDialog.h"
#include "commands/shellexecute/ShellExecEnvBulkAddDialog.h"
#include "gui/FolderDialog.h"
#include "icon/IconLabel.h"
#include "commands/core/CommandRepository.h"
#include "utility/ShortcutFile.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace shellexecute {

struct SettingPageEnv::PImpl
{
	// メッセージ欄
	CString mMessage;
	std::map<CString, CString> mEnviron;
};


SettingPageEnv::SettingPageEnv(CWnd* parentWnd) : 
	SettingPage(_T("環境変数"), IDD_SHELLEXEC_ENVLIST, parentWnd),
	in(new PImpl)
{
}

SettingPageEnv::~SettingPageEnv()
{
}

void SettingPageEnv::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
}

BEGIN_MESSAGE_MAP(SettingPageEnv, SettingPage)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(IDC_BUTTON_BULKADD, OnButtonBulkAdd)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ENVIRON, OnNotifyItemChanged)
	ON_NOTIFY(NM_CLICK, IDC_LIST_ENVIRON, OnNotifyItemClick)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ENVIRON, OnNotifyItemDblClk)
END_MESSAGE_MAP()


BOOL SettingPageEnv::OnInitDialog()
{
	__super::OnInitDialog();

	InitListCtrl();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void SettingPageEnv::InitListCtrl()
{
	auto listWnd = (CListCtrl*)GetDlgItem(IDC_LIST_ENVIRON);
	ASSERT(listWnd);

	// 行全体を選択
	listWnd->SetExtendedStyle(listWnd->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	// 列を挿入
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)_T("環境変数名"));
	lvc.cx = 120;
	lvc.fmt = LVCFMT_LEFT;
	listWnd->InsertColumn(0,&lvc);

	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)_T("環境変数値"));
	lvc.cx = 240;
	lvc.fmt = LVCFMT_LEFT;
	listWnd->InsertColumn(1,&lvc);
}

bool SettingPageEnv::EditItem(int index)
{
	CListCtrl* listWnd = (CListCtrl*)GetDlgItem(IDC_LIST_ENVIRON);

	auto name = listWnd->GetItemText(index, 0);
	auto value = listWnd->GetItemText(index, 1);

	// 編集ダイアログを表示
	ValueEditDialog dlg(this);
	dlg.SetName(name);
	dlg.SetValue(value);
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// 編集後の値を取得し、リストに戻す
	auto newName = dlg.GetName();
	auto newValue = dlg.GetValue();
	listWnd->SetItemText(index, 0, newName);
	listWnd->SetItemText(index, 1, newValue);

	in->mEnviron[newName] = newValue;
	return true;
}

bool SettingPageEnv::UpdateStatus()
{
	in->mMessage.Empty();

	// リスト上の選択項目がない場合は編集と削除を許可しない
	CListCtrl* listWnd = (CListCtrl*)GetDlgItem(IDC_LIST_ENVIRON);
	POSITION pos = listWnd->GetFirstSelectedItemPosition();
	bool hasSelection = pos != nullptr;
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(hasSelection);
	GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(hasSelection);

	EnalbleOKButton();

	return true;
}

void SettingPageEnv::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}


HBRUSH SettingPageEnv::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (::utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = in->mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

BOOL SettingPageEnv::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL SettingPageEnv::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void SettingPageEnv::OnEnterSettings()
{
	auto param = (CommandParam*)GetParam();

	in->mEnviron = param->mEnviron;

	CListCtrl* listWnd = (CListCtrl*)GetDlgItem(IDC_LIST_ENVIRON);
	ASSERT(listWnd);
	listWnd->DeleteAllItems();

	// 要素をリストに登録
	int pos = 0;
	for (auto& item : in->mEnviron) {
		int index = listWnd->InsertItem(pos, item.first);
		listWnd->SetItemText(index, 1, item.second);
	}
}

bool SettingPageEnv::GetHelpPageId(CString& id)
{
	id = _T("ShellExecEnvList");
	return true;
}

void SettingPageEnv::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	// Note : このページで設定するパラメータだけ書き戻す
	auto param = (CommandParam*)GetParam();
	param->mEnviron = in->mEnviron;

	__super::OnOK();
}

void SettingPageEnv::OnButtonAdd()
{
	// 追加のための編集ダイアログを表示
	ValueEditDialog dlg(this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	// 追加された名前と値を取得
	auto newName = dlg.GetName();
	auto newValue = dlg.GetValue();

	CListCtrl* listWnd = (CListCtrl*)GetDlgItem(IDC_LIST_ENVIRON);
	int count = listWnd->GetItemCount();

	// 更新する要素のインデックス値
	int editItemIndex = count;

	// 重複チェック
	auto it = in->mEnviron.find(newName);
	if (it != in->mEnviron.end()) {
		auto msg = fmt::format(_T("{} はすでに登録されています。値を置き換えますか?"), (LPCTSTR)newName);
		int yesno = AfxMessageBox(msg.c_str(), MB_YESNO | MB_ICONQUESTION);
		if (yesno != IDYES) {
			// 更新しない
			return;
		}

		// 重複する要素のリスト上の位置を探す
		for (int i = 0; i < count; ++i) {
			auto name = listWnd->GetItemText(i, 0);
			if (name != newName) {
				continue;
			}
			editItemIndex = i;
			break;
		}
		// 既存の変数を更新
		listWnd->SetItemText(editItemIndex, 0, newName);
	}
	else {
		// 変数を追加
		listWnd->InsertItem(editItemIndex, newName);
	}

	// 更新された値をリストに反映する
	listWnd->SetItemText(editItemIndex, 1, newValue);
	in->mEnviron[newName] = newValue;
}

void SettingPageEnv::OnButtonEdit()
{
	// 選択している項目の現在の値を取得
	CListCtrl* listWnd = (CListCtrl*)GetDlgItem(IDC_LIST_ENVIRON);
	POSITION pos = listWnd->GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return;
	}

	int itemIndex = listWnd->GetNextSelectedItem(pos);
	EditItem(itemIndex);
}

void SettingPageEnv::OnButtonDelete()
{
	// 選択している項目の現在の値を取得
	CListCtrl* listWnd = (CListCtrl*)GetDlgItem(IDC_LIST_ENVIRON);
	POSITION pos = listWnd->GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return;
	}

	// リストから項目を削除
	int itemIndex = listWnd->GetNextSelectedItem(pos);
	auto name = listWnd->GetItemText(itemIndex, 0);

	listWnd->DeleteItem(itemIndex);
	auto it = in->mEnviron.find(name);
	if (it != in->mEnviron.end()) {
		in->mEnviron.erase(it);
	}
}

void SettingPageEnv::OnButtonBulkAdd()
{
	// 追加のための編集ダイアログを表示
	BulkAddDialog dlg(this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	// 追加された名前と値を取得
	BulkAddDialog::ItemList items;
	dlg.GetItems(items);

	CListCtrl* listWnd = (CListCtrl*)GetDlgItem(IDC_LIST_ENVIRON);
	int count = listWnd->GetItemCount();

	// 名前と位置の対応表を作成する
	std::map<CString, int> itemIndexMap;
	for (int i = 0; i < count; ++i) {
		auto name = listWnd->GetItemText(i, 0);
		itemIndexMap[name] = i;
	}

	for (const auto& item : items) {
		const auto& name = item.first;
		const auto& value = item.second;

		auto it = itemIndexMap.find(name);
		bool isExist = it != itemIndexMap.end();

		// 値が存在する場合は上書き、存在しない場合は末尾に追加
		if (isExist) {
			int itemIndex = it->second;
			listWnd->SetItemText(itemIndex, 0, name);
			listWnd->SetItemText(itemIndex, 1, value);
		}
		else {
			int itemIndex = count++;
			listWnd->InsertItem(itemIndex, name);
			listWnd->SetItemText(itemIndex, 1, value);
		}
		in->mEnviron[name] = value;
	}
}

void SettingPageEnv::OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	UpdateStatus();
	UpdateData(FALSE);
	*pResult = 0;
}

void SettingPageEnv::OnNotifyItemClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	*pResult = 0;
}


void SettingPageEnv::OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;

	int index = nm->iItem;
	if (index < 0 || in->mEnviron.size() <= index) {
		return;
	}

	if (EditItem(index) == false) {
		return;
	}
	UpdateStatus();
	UpdateData(FALSE);
}


}}} // end of namespace launcherapp::commands::shellexecute
