#include "pch.h"
#include "framework.h"
#include "ExcludePathPage.h"
#include "gui/FolderDialog.h"
#include "setting/Settings.h"
#include "utility/LocalPathResolver.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct ExcludePathPage::PImpl
{
	CListCtrl* GetPathListWnd() {
		ASSERT(mListPath);
		return mListPath;
	}
	CListCtrl* mListPath = nullptr;
	std::vector<CString> mExcludePaths;
};

ExcludePathPage::ExcludePathPage(CWnd* parentWnd) : 
	SettingPage(_T("除外するファイル"), IDD_APPSETTING_EXCLUDEFILE, parentWnd),
	in(new PImpl)
{
}

ExcludePathPage::~ExcludePathPage()
{
}

void ExcludePathPage::SwapItem(int srcIndex, int dstIndex)
{
	auto listPath = in->GetPathListWnd();

	CString srcText = listPath->GetItemText(srcIndex, 0);
	CString dstText = listPath->GetItemText(dstIndex, 0);
	listPath->SetItemText(srcIndex, 0, dstText);
	listPath->SetItemText(dstIndex, 0, srcText);

	// 選択
	listPath->SetItemState(srcIndex, 0, LVIS_SELECTED);
	listPath->SetItemState(dstIndex, LVIS_SELECTED, LVIS_SELECTED);

	std::swap(in->mExcludePaths[srcIndex], in->mExcludePaths[dstIndex]);
}

BOOL ExcludePathPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL ExcludePathPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void ExcludePathPage::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();

	TCHAR key[64];
	int index = 1;

	settingsPtr->Set(_T("Soyokaze:ExcludePathCount"), (int)in->mExcludePaths.size());
	for (const auto& path : in->mExcludePaths) {

		_stprintf_s(key, _T("Soyokaze:ExcludePath%d"), index++);
		settingsPtr->Set(key, path);
	}

	__super::OnOK();
}

void ExcludePathPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ExcludePathPage, SettingPage)
	ON_COMMAND(IDC_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PATH, OnNotifyItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PATH, OnNotifyItemDblClk)
END_MESSAGE_MAP()


BOOL ExcludePathPage::OnInitDialog()
{
	__super::OnInitDialog();

	in->mListPath = (CListCtrl*)GetDlgItem(IDC_LIST_PATH);

	auto listPath = in->GetPathListWnd();
	listPath->SetExtendedStyle(listPath->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	CString strHeader(_T("Path"));

	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 370;
	lvc.fmt = LVCFMT_LEFT;
	listPath->InsertColumn(0,&lvc);


	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ExcludePathPage::UpdateStatus()
{
	auto listPath = in->GetPathListWnd();

	POSITION pos = listPath->GetFirstSelectedItemPosition();
	bool hasSelect = pos != NULL;

	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(hasSelect);
	GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(hasSelect);

	return true;
}

void ExcludePathPage::OnEnterSettings()
{
	auto listPath = in->GetPathListWnd();
	listPath->DeleteAllItems();

	auto settingsPtr = (Settings*)GetParam();

	std::vector<CString> paths;

	TCHAR key[64];
	int n = settingsPtr->Get(_T("Soyokaze:ExcludePathCount"), 0);
	for (int index = 0; index < n; ++index) {
		_stprintf_s(key, _T("Soyokaze:ExcludePath%d"), index+1);
		CString path = settingsPtr->Get(key, _T(""));
		paths.push_back(path);

		listPath->InsertItem(index++, path);
	}

	in->mExcludePaths.swap(paths);


}

void ExcludePathPage::OnButtonAdd()
{
	TCHAR path[MAX_PATH_NTFS];
	GetModuleFileName(nullptr, path, MAX_PATH_NTFS);
	CFileDialog dlg(TRUE, NULL, path, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	auto listPath = in->GetPathListWnd();
	in->mExcludePaths.push_back(dlg.GetPathName());
	listPath->InsertItem(listPath->GetItemCount(), dlg.GetPathName());
}

void ExcludePathPage::OnButtonEdit()
{
	auto listPath = in->GetPathListWnd();
	POSITION pos = listPath->GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return ;
	}
	int itemIndex = listPath->GetNextSelectedItem(pos);
	auto& path = in->mExcludePaths[itemIndex];

	CFileDialog dlg(TRUE, NULL, path, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}
	path = dlg.GetPathName();
	listPath->SetItemText(itemIndex, 0, path);
}

void ExcludePathPage::OnButtonDelete()
{
	auto listPath = in->GetPathListWnd();
	POSITION pos = listPath->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listPath->GetNextSelectedItem(pos);
	listPath->DeleteItem(itemIndex);
	in->mExcludePaths.erase(in->mExcludePaths.begin() + itemIndex);

	if (itemIndex < listPath->GetItemCount()) {
		listPath->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
	}
	else {
		itemIndex--;
		if (0 <= itemIndex && itemIndex < listPath->GetItemCount()) {
			listPath->SetItemState(itemIndex, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

	UpdateStatus();
	UpdateData(FALSE);
}

void ExcludePathPage::OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateStatus();
	UpdateData(FALSE);
	*pResult = 0;
}

void ExcludePathPage::OnNotifyItemDblClk(
		NMHDR *pNMHDR,
	 	LRESULT *pResult
)
{
	auto listPath = in->GetPathListWnd();

	*pResult = 0;

	NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;

	int index = nm->iItem;
	if (index < 0 || (int)in->mExcludePaths.size() <= index) {
		return;
	}

	auto& path = in->mExcludePaths[index];

	CFileDialog dlg(TRUE, NULL, path, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	path = dlg.GetPathName();
	listPath->SetItemText(index, 0, path);
}

