#include "pch.h"
#include "framework.h"
#include "ExecSettingDialog.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


ExecSettingDialog::ExecSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("実行"), IDD_EXECSETTING, parentWnd)
{
}

ExecSettingDialog::~ExecSettingDialog()
{
}

BOOL ExecSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL ExecSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void ExecSettingDialog::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("Soyokaze:UseFiler"), (bool)mIsUseExternalFiler);
	settingsPtr->Set(_T("Soyokaze:FilerPath"), mFilerPath);
	settingsPtr->Set(_T("Soyokaze:FilerParam"), mFilerParam);
	settingsPtr->Set(_T("Soyokaze:IsArrowFilterCommandConcurrentRun"), (bool)mIsArrowFilterConcurrentRun);
	settingsPtr->Set(_T("Soyokaze:IsShowFolderIfCtrlPressed"), (bool)mIsShowFolderIfCtrlPressed);

	__super::OnOK();
}

void ExecSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_USEFILER, mIsUseExternalFiler);
	DDX_Text(pDX, IDC_EDIT_FILERPATH, mFilerPath);
	DDX_Text(pDX, IDC_EDIT_FILERPARAM, mFilerParam);
	DDX_Check(pDX, IDC_CHECK_ALLOWCONCURRENTEXEC_FILTER, mIsArrowFilterConcurrentRun);
	DDX_Check(pDX, IDC_CHECK_SHOWDIR, mIsShowFolderIfCtrlPressed);
}

BEGIN_MESSAGE_MAP(ExecSettingDialog, SettingPage)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE, OnButtonBrowseFile)
	ON_COMMAND(IDC_CHECK_USEFILER, OnCheckUseFilter)
END_MESSAGE_MAP()


BOOL ExecSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// ファイル選択ボタン(絵文字にする)
	GetDlgItem(IDC_BUTTON_BROWSEFILE)->SetWindowTextW(L"\U0001F4C4");

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ExecSettingDialog::UpdateStatus()
{
	// 外部ファイラーを使わない場合は関連する設定をグレーアウトする
	GetDlgItem(IDC_EDIT_FILERPATH)->EnableWindow(mIsUseExternalFiler);
	GetDlgItem(IDC_EDIT_FILERPARAM)->EnableWindow(mIsUseExternalFiler);
	GetDlgItem(IDC_BUTTON_BROWSEFILE)->EnableWindow(mIsUseExternalFiler);

	return true;
}

void ExecSettingDialog::OnButtonBrowseFile()
{
	UpdateData();

	CString filterStr((LPCTSTR)IDS_FILTER_EXE);
	CFileDialog dlg(TRUE, NULL, mFilerPath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mFilerPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void ExecSettingDialog::OnCheckUseFilter()
{
	UpdateData();
	UpdateStatus();
}

void ExecSettingDialog::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();
	mIsUseExternalFiler = settingsPtr->Get(_T("Soyokaze:UseFiler"), false);
	mFilerPath = settingsPtr->Get(_T("Soyokaze:FilerPath"), _T(""));
	mFilerParam = settingsPtr->Get(_T("Soyokaze:FilerParam"), _T(""));
	mIsArrowFilterConcurrentRun = settingsPtr->Get(_T("Soyokaze:IsArrowFilterCommandConcurrentRun"), false);
	mIsShowFolderIfCtrlPressed = settingsPtr->Get(_T("Soyokaze:IsShowFolderIfCtrlPressed"), true);

}
