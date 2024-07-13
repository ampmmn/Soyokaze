#include "pch.h"
#include "SelectCommandTypeDialog.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SelectCommandTypeDialog::SelectCommandTypeDialog() :
	launcherapp::gui::SinglePageDialog(IDD_SELECTCOMMANDTYPE)
{
	SetHelpPageId(_T("SelectCommandType"));
}

SelectCommandTypeDialog::~SelectCommandTypeDialog()
{
}

int SelectCommandTypeDialog::AddType(
	const CString displayName,
	const CString& description,
	LPARAM itemData
)
{
	ITEM newItem;
	newItem.mDisplayName = displayName;
	newItem.mDescription = description;
	newItem.mItemData = itemData;

	mItems.push_back(newItem);

	return (int)mItems.size()-1;
}

LPARAM SelectCommandTypeDialog::GetSelectedItem()
{
	if (mSelIndex < 0 || mItems.size() <= (size_t)mSelIndex) {
		return (LPARAM)nullptr;
	}
	return mItems[mSelIndex].mItemData;
}

void SelectCommandTypeDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, mDescriptionStr);
}

BEGIN_MESSAGE_MAP(SelectCommandTypeDialog, launcherapp::gui::SinglePageDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TYPES, OnNotifyItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TYPES, OnNotifyItemDblClick)
END_MESSAGE_MAP()

BOOL SelectCommandTypeDialog::OnInitDialog()
{
	__super::OnInitDialog();

	CListCtrl* listTypes = (CListCtrl*)GetDlgItem(IDC_LIST_TYPES);
	ASSERT(listTypes);

	// リスト　スタイル変更
	listTypes->SetExtendedStyle(listTypes->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	// ヘッダー追加
	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	CString strHeader;
	strHeader.LoadString(IDS_NAME);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 120;
	lvc.fmt = LVCFMT_LEFT;
	listTypes->InsertColumn(0,&lvc);

	strHeader.LoadString(IDS_DESCRIPTION);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 300;
	lvc.fmt = LVCFMT_LEFT;
	listTypes->InsertColumn(1,&lvc);

	int listItemCount = 0;
	for (auto& item : mItems) {
		int index = listItemCount++;
		listTypes->InsertItem(index, item.mDisplayName, 0);
		listTypes->SetItemText(index, 1, item.mDescription);
	}

	if (mItems.size() > 0) {
		listTypes->SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		mSelIndex = 0;
	}

	UpdateStatus();

	listTypes->SetFocus();

	return TRUE;
}

void SelectCommandTypeDialog::OnOK()
{
	CListCtrl* listTypes = (CListCtrl*)GetDlgItem(IDC_LIST_TYPES);
	ASSERT(listTypes);

	POSITION pos = listTypes->GetFirstSelectedItemPosition();
	if (pos != NULL) {
		mSelIndex = listTypes->GetNextSelectedItem(pos);
	}
	else {
		mSelIndex = -1;
	}

	__super::OnOK();
}

bool SelectCommandTypeDialog::UpdateStatus()
{
	CListCtrl* listTypes = (CListCtrl*)GetDlgItem(IDC_LIST_TYPES);
	ASSERT(listTypes);

	POSITION pos = listTypes->GetFirstSelectedItemPosition();

	BOOL isSelected = pos != nullptr;

	GetDlgItem(IDOK)->EnableWindow(isSelected);

	// 説明を更新
	if (isSelected) {
		int itemIndex = listTypes->GetNextSelectedItem(pos);
		mDescriptionStr = mItems[itemIndex].mDescription;
	}
	else {
		mDescriptionStr.Empty();
	}

	UpdateData(FALSE);
	return true;
}

void SelectCommandTypeDialog::OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	UNREFERENCED_PARAMETER(pResult);

	UpdateStatus();
}

void SelectCommandTypeDialog::OnNotifyItemDblClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	UNREFERENCED_PARAMETER(pResult);

	OnOK();
}

INT_PTR SelectCommandTypeDialog::DoModal()
{
	if (mItems.empty()) {
		// 要素がなければ選択しようがない(してもしょうがない)
		return IDCANCEL;
	}
	if (mItems.size() == 1) {
		// 要素が1つの場合は選択するまでもないので表示しない
		mSelIndex = 0;
		return IDOK;
	}

	return __super::DoModal();
}
