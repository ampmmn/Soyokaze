#include "pch.h"
#include "framework.h"
#include "ExecSettingDialog.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


ExecSettingDialog::ExecSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("実行"), IDD_EXECSETTING, parentWnd),
	mIsShowFolderIfCtrlPressed(true),
	mIsUseExternalFiler(false),
	mIsEnablePathFind(true),
	mDefaultActionIndex(0)
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
	UpdateData();

	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("Soyokaze:UseFiler"), (bool)mIsUseExternalFiler);
	settingsPtr->Set(_T("Soyokaze:FilerPath"), mFilerPath);
	settingsPtr->Set(_T("Soyokaze:FilerParam"), mFilerParam);
	settingsPtr->Set(_T("Soyokaze:IsShowFolderIfCtrlPressed"), (bool)mIsShowFolderIfCtrlPressed);
	settingsPtr->Set(_T("Soyokaze:IsEnablePathFind"), (bool)mIsEnablePathFind);

	CString defAction;
	if (mDefaultActionIndex == 1) {
		// FIXME: 今後機能を拡張するようなことがあれば別クラスに処理を移すこと
		defAction = _T("copy");
	}
	else if (mDefaultActionIndex == 2) {
		defAction = _T("register");
	}
	else {
		// 何もしない
		defAction = _T("");
	}

	settingsPtr->Set(_T("Launcher:DefaultActionType"), defAction); 

	__super::OnOK();
}

void ExecSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_USEFILER, mIsUseExternalFiler);
	DDX_Text(pDX, IDC_EDIT_FILERPATH, mFilerPath);
	DDX_Text(pDX, IDC_EDIT_FILERPARAM, mFilerParam);
	DDX_Check(pDX, IDC_CHECK_ENABLEPATHFIND, mIsEnablePathFind);
	DDX_Check(pDX, IDC_CHECK_SHOWDIR, mIsShowFolderIfCtrlPressed);
	DDX_CBIndex(pDX, IDC_COMBO_DEFAULTACTION, mDefaultActionIndex);
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
	mIsShowFolderIfCtrlPressed = settingsPtr->Get(_T("Soyokaze:IsShowFolderIfCtrlPressed"), true);
	mIsEnablePathFind = settingsPtr->Get(_T("Soyokaze:IsEnablePathFind"), true);

	CString defAction = settingsPtr->Get(_T("Launcher:DefaultActionType"), _T("register")); 
	if (defAction == _T("copy")) {
		// FIXME: 今後機能を拡張するようなことがあれば別クラスに処理を移すこと
		mDefaultActionIndex = 1;
	}
	else if (defAction == _T("register")) {
		mDefaultActionIndex = 2;
	}
	else {
		// なにもしない
		mDefaultActionIndex = 0;
	}

}

bool ExecSettingDialog::GetHelpPageId(CString& id)
{
	id = _T("ExecuteSetting");
	return true;
}

