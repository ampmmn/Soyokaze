#include "pch.h"
#include "framework.h"
#include "AppSettingPathPage.h"
#include "gui/FolderDialog.h"
#include "setting/Settings.h"
#include "utility/LocalPathResolver.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct AppSettingPathPage::PImpl
{
	CListCtrl* GetPathListWnd() {
		ASSERT(mListPath);
		return mListPath;
	}
	CListCtrl* mListPath = nullptr;
	std::vector<CString> mAdditionalPaths;
};

AppSettingPathPage::AppSettingPathPage(CWnd* parentWnd) : 
	SettingPage(_T("パス"), IDD_APPSETTING_PATH, parentWnd),
	in(new PImpl)
{
}

AppSettingPathPage::~AppSettingPathPage()
{
}

void AppSettingPathPage::SwapItem(int srcIndex, int dstIndex)
{
	auto listPath = in->GetPathListWnd();

	CString srcText = listPath->GetItemText(srcIndex, 0);
	CString dstText = listPath->GetItemText(dstIndex, 0);
	listPath->SetItemText(srcIndex, 0, dstText);
	listPath->SetItemText(dstIndex, 0, srcText);

	// 選択
	listPath->SetItemState(srcIndex, 0, LVIS_SELECTED);
	listPath->SetItemState(dstIndex, LVIS_SELECTED, LVIS_SELECTED);

	std::swap(in->mAdditionalPaths[srcIndex], in->mAdditionalPaths[dstIndex]);
}

BOOL AppSettingPathPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL AppSettingPathPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void AppSettingPathPage::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();

	TCHAR key[64];
	int index = 1;

	settingsPtr->Set(_T("Soyokaze:AdditionalPathCount"), (int)in->mAdditionalPaths.size());
	for (const auto& path : in->mAdditionalPaths) {

		_stprintf_s(key, _T("Soyokaze:AdditionalPath%d"), index++);
		settingsPtr->Set(key, path);
	}

	__super::OnOK();
}

void AppSettingPathPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(AppSettingPathPage, SettingPage)
	ON_COMMAND(IDC_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(IDC_BUTTON_UP, OnButtonUp)
	ON_COMMAND(IDC_BUTTON_DOWN, OnButtonDown)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PATH, OnNotifyItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PATH, OnNotifyItemDblClk)
END_MESSAGE_MAP()


BOOL AppSettingPathPage::OnInitDialog()
{
	__super::OnInitDialog();

	in->mListPath = (CListCtrl*)GetDlgItem(IDC_LIST_PATH);

	auto listSysPath = (CListCtrl*)GetDlgItem(IDC_LIST_ENVPATH);
	ASSERT(listSysPath);
	listSysPath->SetExtendedStyle(listSysPath->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	auto listPath = in->GetPathListWnd();
	listPath->SetExtendedStyle(listPath->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	CString strHeader(_T("Directory"));

	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 370;
	lvc.fmt = LVCFMT_LEFT;
	listSysPath->InsertColumn(0,&lvc);
	listPath->InsertColumn(0,&lvc);

	// システムパスを取得
	int index = 0;
	std::vector<CString> systemPaths;
	launcherapp::utility::LocalPathResolver::GetSystemPath(systemPaths);
	for (auto& path : systemPaths) {
		listSysPath->InsertItem(index, path);
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool AppSettingPathPage::UpdateStatus()
{
	auto listPath = in->GetPathListWnd();

	POSITION pos = listPath->GetFirstSelectedItemPosition();
	bool hasSelect = pos != NULL;

	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(hasSelect);
	GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(hasSelect);
	if (hasSelect) {
		int itemIndex = listPath->GetNextSelectedItem(pos);
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(itemIndex > 0);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(itemIndex < listPath->GetItemCount()-1);
	}
	else {
		GetDlgItem(IDC_BUTTON_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(FALSE);
	}

	return true;
}

void AppSettingPathPage::OnEnterSettings()
{
	auto listPath = in->GetPathListWnd();
	listPath->DeleteAllItems();

	auto settingsPtr = (Settings*)GetParam();

	std::vector<CString> paths;

	TCHAR key[64];
	int n = settingsPtr->Get(_T("Soyokaze:AdditionalPathCount"), 0);
	for (int index = 0; index < n; ++index) {
		_stprintf_s(key, _T("Soyokaze:AdditionalPath%d"), index+1);
		CString path = settingsPtr->Get(key, _T(""));
		paths.push_back(path);

		listPath->InsertItem(index++, path);
	}

	in->mAdditionalPaths.swap(paths);


}

bool AppSettingPathPage::GetHelpPageId(CString& id)
{
	id = _T("ExecutePathSetting");
	return true;
}


void AppSettingPathPage::OnButtonAdd()
{
	TCHAR path[MAX_PATH_NTFS];
	GetModuleFileName(nullptr, path, MAX_PATH_NTFS);
	CFolderDialog dlg(_T("ディレクトリの選択"), path, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	auto listPath = in->GetPathListWnd();
	in->mAdditionalPaths.push_back(dlg.GetPathName());
	listPath->InsertItem(listPath->GetItemCount(), dlg.GetPathName());
}

void AppSettingPathPage::OnButtonEdit()
{
	auto listPath = in->GetPathListWnd();
	POSITION pos = listPath->GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return ;
	}
	int itemIndex = listPath->GetNextSelectedItem(pos);
	auto& path = in->mAdditionalPaths[itemIndex];

	CFolderDialog dlg(_T("ディレクトリの選択"), path, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}
	path = dlg.GetPathName();
	listPath->SetItemText(itemIndex, 0, path);
}

void AppSettingPathPage::OnButtonDelete()
{
	auto listPath = in->GetPathListWnd();
	POSITION pos = listPath->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listPath->GetNextSelectedItem(pos);
	listPath->DeleteItem(itemIndex);
	in->mAdditionalPaths.erase(in->mAdditionalPaths.begin() + itemIndex);

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

void AppSettingPathPage::OnButtonUp()
{
	auto listPath = in->GetPathListWnd();
	POSITION pos = listPath->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listPath->GetNextSelectedItem(pos);
	if (itemIndex == 0) {
		return;
	}
	SwapItem(itemIndex, itemIndex-1);

	UpdateStatus();
	UpdateData(FALSE);
}

void AppSettingPathPage::OnButtonDown()
{
	auto listPath = in->GetPathListWnd();
	POSITION pos = listPath->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listPath->GetNextSelectedItem(pos);
	if (itemIndex == listPath->GetItemCount()-1) {
		return;
	}
	SwapItem(itemIndex, itemIndex+1);

	UpdateStatus();
	UpdateData(FALSE);
}

void AppSettingPathPage::OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateStatus();
	UpdateData(FALSE);
	*pResult = 0;
}

void AppSettingPathPage::OnNotifyItemDblClk(
		NMHDR *pNMHDR,
	 	LRESULT *pResult
)
{
	auto listPath = in->GetPathListWnd();

	*pResult = 0;

	NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;

	int index = nm->iItem;
	if (index < 0 || (int)in->mAdditionalPaths.size() <= index) {
		return;
	}

	auto& path = in->mAdditionalPaths[index];

	CFolderDialog dlg(_T("ディレクトリの選択"), path, this);

	if (dlg.DoModal() != IDOK) {
		return ;
	}

	path = dlg.GetPathName();
	listPath->SetItemText(index, 0, path);
}

