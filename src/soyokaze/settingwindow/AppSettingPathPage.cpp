#include "pch.h"
#include "framework.h"
#include "AppSettingPathPage.h"
#include "gui/FolderDialog.h"
#include "setting/Settings.h"
#include "utility/LocalPathResolver.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 
class PathSettingDialog : public CDialog
{
public:
	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	void SwapItem(int srcIndex, int dstIndex);
	bool UpdateStatus();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

	CListCtrl* GetPathListWnd() {
		ASSERT(mListPath);
		return mListPath;
	}

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult);

public:
	CListCtrl* mListPath{nullptr};
	std::vector<CString> mAdditionalPaths;
	Settings* mSettingsPtr{nullptr};
};

void PathSettingDialog::SwapItem(int srcIndex, int dstIndex)
{
	auto listPath = GetPathListWnd();

	CString srcText = listPath->GetItemText(srcIndex, 0);
	CString dstText = listPath->GetItemText(dstIndex, 0);
	listPath->SetItemText(srcIndex, 0, dstText);
	listPath->SetItemText(dstIndex, 0, srcText);

	// 選択
	listPath->SetItemState(srcIndex, 0, LVIS_SELECTED);
	listPath->SetItemState(dstIndex, LVIS_SELECTED, LVIS_SELECTED);

	std::swap(mAdditionalPaths[srcIndex], mAdditionalPaths[dstIndex]);
}

bool PathSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool PathSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void PathSettingDialog::OnOK()
{
	auto settingsPtr = mSettingsPtr;

	TCHAR key[64];
	int index = 1;

	settingsPtr->Set(_T("Soyokaze:AdditionalPathCount"), (int)mAdditionalPaths.size());
	for (const auto& path : mAdditionalPaths) {

		_stprintf_s(key, _T("Soyokaze:AdditionalPath%d"), index++);
		settingsPtr->Set(key, path);
	}

	__super::OnOK();
}

void PathSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(PathSettingDialog, CDialog)
	ON_COMMAND(IDC_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(IDC_BUTTON_UP, OnButtonUp)
	ON_COMMAND(IDC_BUTTON_DOWN, OnButtonDown)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PATH, OnNotifyItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PATH, OnNotifyItemDblClk)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL PathSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mListPath = (CListCtrl*)GetDlgItem(IDC_LIST_PATH);

	auto listSysPath = (CListCtrl*)GetDlgItem(IDC_LIST_ENVPATH);
	ASSERT(listSysPath);
	listSysPath->SetExtendedStyle(listSysPath->GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	auto listPath = GetPathListWnd();
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

bool PathSettingDialog::UpdateStatus()
{
	auto listPath = GetPathListWnd();

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

void PathSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	auto listPath = GetPathListWnd();
	listPath->DeleteAllItems();

	std::vector<CString> paths;

	TCHAR key[64];
	int n = settingsPtr->Get(_T("Soyokaze:AdditionalPathCount"), 0);
	for (int index = 0; index < n; ++index) {
		_stprintf_s(key, _T("Soyokaze:AdditionalPath%d"), index+1);
		CString path = settingsPtr->Get(key, _T(""));
		paths.push_back(path);

		listPath->InsertItem(index++, path);
	}

	mAdditionalPaths.swap(paths);


}

void PathSettingDialog::OnButtonAdd()
{
	Path path(Path::MODULEFILEDIR);
	CFolderDialog dlg(_T("ディレクトリの選択"), path, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	auto listPath = GetPathListWnd();
	mAdditionalPaths.push_back(dlg.GetPathName());
	listPath->InsertItem(listPath->GetItemCount(), dlg.GetPathName());
}

void PathSettingDialog::OnButtonEdit()
{
	auto listPath = GetPathListWnd();
	POSITION pos = listPath->GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return ;
	}
	int itemIndex = listPath->GetNextSelectedItem(pos);
	auto& path = mAdditionalPaths[itemIndex];

	CFolderDialog dlg(_T("ディレクトリの選択"), path, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}
	path = dlg.GetPathName();
	listPath->SetItemText(itemIndex, 0, path);
}

void PathSettingDialog::OnButtonDelete()
{
	auto listPath = GetPathListWnd();
	POSITION pos = listPath->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listPath->GetNextSelectedItem(pos);
	listPath->DeleteItem(itemIndex);
	mAdditionalPaths.erase(mAdditionalPaths.begin() + itemIndex);

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

void PathSettingDialog::OnButtonUp()
{
	auto listPath = GetPathListWnd();
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

void PathSettingDialog::OnButtonDown()
{
	auto listPath = GetPathListWnd();
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

void PathSettingDialog::OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	UpdateStatus();
	UpdateData(FALSE);
	*pResult = 0;
}

void PathSettingDialog::OnNotifyItemDblClk(
		NMHDR *pNMHDR,
	 	LRESULT *pResult
)
{
	auto listPath = GetPathListWnd();

	*pResult = 0;

	NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;

	int index = nm->iItem;
	if (index < 0 || (int)mAdditionalPaths.size() <= index) {
		return;
	}

	auto& path = mAdditionalPaths[index];

	CFolderDialog dlg(_T("ディレクトリの選択"), path, this);

	if (dlg.DoModal() != IDOK) {
		return ;
	}

	path = dlg.GetPathName();
	listPath->SetItemText(index, 0, path);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPagePathPage::PImpl
{
	PathSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPagePathPage)

AppSettingPagePathPage::AppSettingPagePathPage() : 
	AppSettingPageBase(_T("実行"), _T("パス")),
	in(new PImpl)
{
}

AppSettingPagePathPage::~AppSettingPagePathPage()
{
}

// ウインドウを作成する
bool AppSettingPagePathPage::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_PATH, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPagePathPage::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPagePathPage::GetOrder()
{
	return 10;
}
// 
bool AppSettingPagePathPage::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPagePathPage::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPagePathPage::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPagePathPage::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPagePathPage::GetHelpPageId(String& id)
{
	id = "ExecutePathSetting";
	return true;
}

