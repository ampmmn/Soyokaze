#include "pch.h"
#include "framework.h"
#include "ExcludeTargetPage.h"
#include "control/FolderDialog.h"
#include "setting/Settings.h"
#include "utility/Regex.h"
#include "utility/LocalPathResolver.h"
#include "utility/Path.h"
#include "resource.h"
#include <afxvslistbox.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class DisplayNamePatternListBox : public CVSListBox
{
protected:
	/**
	  編集後のパターンを検証し、無効な値の確定を取り消す
	  @param[in] label 編集されたパターン
	*/
	void OnEndEditLabel(LPCTSTR label) override;
};

void DisplayNamePatternListBox::OnEndEditLabel(LPCTSTR label)
{
	if (label == nullptr || *label == _T('\0')) {
		CVSListBox::OnEndEditLabel(label);
		return;
	}

	launcherapp::utility::Regex regex;
	if (regex.Compile(label, false) == false) {
		CString message((LPCTSTR)IDS_ERR_INVALIDREGEXP);
		message += _T("\n");
		CStringA error(regex.GetError().c_str());
		message += (CString)error;
		message += _T("\n");
		message += label;
		AfxMessageBox(message);

		CVSListBox::OnEndEditLabel(nullptr);
		return;
	}

	CVSListBox::OnEndEditLabel(label);
}

class ExcludeTargetPage : public CDialog
{
public:

	void SwapItem(int srcIndex, int dstIndex);
	bool UpdateStatus();

	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

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
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult);

public:
	CListCtrl* mListPath{nullptr};
	DisplayNamePatternListBox mDisplayNamePatternList;
	std::vector<CString> mExcludePaths;

	Settings* mSettingsPtr{nullptr};
};

void ExcludeTargetPage::SwapItem(int srcIndex, int dstIndex)
{
	auto listPath = GetPathListWnd();

	CString srcText = listPath->GetItemText(srcIndex, 0);
	CString dstText = listPath->GetItemText(dstIndex, 0);
	listPath->SetItemText(srcIndex, 0, dstText);
	listPath->SetItemText(dstIndex, 0, srcText);

	// 選択
	listPath->SetItemState(srcIndex, 0, LVIS_SELECTED);
	listPath->SetItemState(dstIndex, LVIS_SELECTED, LVIS_SELECTED);

	std::swap(mExcludePaths[srcIndex], mExcludePaths[dstIndex]);
}

bool ExcludeTargetPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool ExcludeTargetPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void ExcludeTargetPage::OnOK()
{
	auto settingsPtr = mSettingsPtr;

	TCHAR key[64];
	int index = 1;

	settingsPtr->Set(_T("Soyokaze:ExcludePathCount"), (int)mExcludePaths.size());
	for (const auto& path : mExcludePaths) {

		_stprintf_s(key, _T("Soyokaze:ExcludePath%d"), index++);
		settingsPtr->Set(key, path);
	}

	settingsPtr->Set(_T("ExcludeTarget:PatternCount"), mDisplayNamePatternList.GetCount());
	for (int index = 0; index < mDisplayNamePatternList.GetCount(); ++index) {
		_stprintf_s(key, _T("ExcludeTarget:Pattern%d"), index);
		settingsPtr->Set(key, mDisplayNamePatternList.GetItemText(index));
	}

	__super::OnOK();
}

void ExcludeTargetPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VSLISTBOX_DISPLAYNAMEPATTERN, mDisplayNamePatternList);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(ExcludeTargetPage, CDialog)
	ON_COMMAND(IDC_BUTTON_ADD, OnButtonAdd)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PATH, OnNotifyItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PATH, OnNotifyItemDblClk)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL ExcludeTargetPage::OnInitDialog()
{
	__super::OnInitDialog();

	mListPath = (CListCtrl*)GetDlgItem(IDC_LIST_PATH);

	auto listPath = GetPathListWnd();
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

bool ExcludeTargetPage::UpdateStatus()
{
	auto listPath = GetPathListWnd();

	POSITION pos = listPath->GetFirstSelectedItemPosition();
	bool hasSelect = pos != NULL;

	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(hasSelect);
	GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(hasSelect);

	return true;
}

void ExcludeTargetPage::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

	auto listPath = GetPathListWnd();
	listPath->DeleteAllItems();
	while (mDisplayNamePatternList.GetCount() > 0) {
		mDisplayNamePatternList.RemoveItem(0);
	}

	std::vector<CString> paths;

	TCHAR key[64];
	int n = settingsPtr->Get(_T("Soyokaze:ExcludePathCount"), 0);
	for (int index = 0; index < n; ++index) {
		_stprintf_s(key, _T("Soyokaze:ExcludePath%d"), index+1);
		CString path = settingsPtr->Get(key, _T(""));
		paths.push_back(path);

		listPath->InsertItem(index, path);
	}

	n = settingsPtr->Get(_T("ExcludeTarget:PatternCount"), 0);
	for (int index = 0; index < n; ++index) {
		_stprintf_s(key, _T("ExcludeTarget:Pattern%d"), index);
		mDisplayNamePatternList.AddItem(settingsPtr->Get(key, _T("")));
	}

	mExcludePaths.swap(paths);


}

void ExcludeTargetPage::OnButtonAdd()
{
	Path path(Path::MODULEFILEPATH);
	CFileDialog dlg(TRUE, NULL, path, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	auto listPath = GetPathListWnd();
	mExcludePaths.push_back(dlg.GetPathName());
	listPath->InsertItem(listPath->GetItemCount(), dlg.GetPathName());
}

void ExcludeTargetPage::OnButtonEdit()
{
	auto listPath = GetPathListWnd();
	POSITION pos = listPath->GetFirstSelectedItemPosition();
	if (pos == nullptr) {
		return ;
	}
	int itemIndex = listPath->GetNextSelectedItem(pos);
	auto& path = mExcludePaths[itemIndex];

	CFileDialog dlg(TRUE, NULL, path, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}
	path = dlg.GetPathName();
	listPath->SetItemText(itemIndex, 0, path);
}

void ExcludeTargetPage::OnButtonDelete()
{
	auto listPath = GetPathListWnd();
	POSITION pos = listPath->GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	int itemIndex = listPath->GetNextSelectedItem(pos);
	listPath->DeleteItem(itemIndex);
	mExcludePaths.erase(mExcludePaths.begin() + itemIndex);

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

void ExcludeTargetPage::OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	UpdateStatus();
	UpdateData(FALSE);
	*pResult = 0;
}

void ExcludeTargetPage::OnNotifyItemDblClk(
		NMHDR *pNMHDR,
	 	LRESULT *pResult
)
{
	auto listPath = GetPathListWnd();

	*pResult = 0;

	NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;

	int index = nm->iItem;
	if (index < 0 || (int)mExcludePaths.size() <= index) {
		return;
	}

	auto& path = mExcludePaths[index];

	CFileDialog dlg(TRUE, NULL, path, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	path = dlg.GetPathName();
	listPath->SetItemText(index, 0, path);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageExcludeTarget::PImpl
{
	ExcludeTargetPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageExcludeTarget)

AppSettingPageExcludeTarget::AppSettingPageExcludeTarget() :
	AppSettingPageBase(_T("実行"), _T("除外する項目")),
	in(new PImpl)
{
}

AppSettingPageExcludeTarget::~AppSettingPageExcludeTarget()
{
}

// ウインドウを作成する
bool AppSettingPageExcludeTarget::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_EXCLUDEFILE, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageExcludeTarget::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageExcludeTarget::GetOrder()
{
	return 30;
}
// 
bool AppSettingPageExcludeTarget::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageExcludeTarget::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageExcludeTarget::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageExcludeTarget::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageExcludeTarget::GetHelpPageId(String& id)
{
	id = "ExcludeFileSetting";
	return true;
}

