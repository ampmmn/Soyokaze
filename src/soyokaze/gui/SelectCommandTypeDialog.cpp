#include "pch.h"
#include "SelectCommandTypeDialog.h"
#include "utility/StringUtil.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SelectCommandTypeDialog::SelectCommandTypeDialog() :
	launcherapp::gui::SinglePageDialog(IDD_SELECTCOMMANDTYPE),
	mSelIndex(-1)
{
	SetHelpPageId("SelectCommandType");
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

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(SelectCommandTypeDialog, launcherapp::gui::SinglePageDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TYPES, OnNotifyItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TYPES, OnNotifyItemDblClick)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL SelectCommandTypeDialog::OnInitDialog()
{
	__super::OnInitDialog();

	CListCtrl* listTypes = (CListCtrl*)GetDlgItem(IDC_LIST_TYPES);
	ASSERT(listTypes);

	// $B%j%9%H!!%9%?%$%kJQ99(B
	listTypes->SetExtendedStyle(listTypes->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	// $B%X%C%@!<DI2C(B
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
	lvc.cx = 450;
	lvc.fmt = LVCFMT_LEFT;
	listTypes->InsertColumn(1,&lvc);

	int listItemCount = 0;
	for (auto& item : mItems) {
		int index = listItemCount++;
		listTypes->InsertItem(index, item.mDisplayName, 0);

		CString desc = launcherapp::utility::GetFirstLine(item.mDescription);
		listTypes->SetItemText(index, 1, desc);
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

	// $B@bL@$r99?7(B
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
		// $BMWAG$,$J$1$l$PA*Br$7$h$&$,$J$$(B($B$7$F$b$7$g$&$,$J$$(B)
		return IDCANCEL;
	}
	if (mItems.size() == 1) {
		// $BMWAG$,(B1$B$D$N>l9g$OA*Br$9$k$^$G$b$J$$$N$GI=<($7$J$$(B
		mSelIndex = 0;
		return IDOK;
	}

	return __super::DoModal();
}
