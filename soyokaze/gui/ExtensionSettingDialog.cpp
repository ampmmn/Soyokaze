#include "pch.h"
#include "framework.h"
#include "ExtensionSettingDialog.h"
#include "Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


ExtensionSettingDialog::ExtensionSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("拡張機能"), IDD_EXTENSIONSETTING, parentWnd),
	mIsEnableCalc(FALSE)

{
}

ExtensionSettingDialog::~ExtensionSettingDialog()
{
}

void ExtensionSettingDialog::OnOK()
{
	UpdateData();


	auto settingsPtr = (Settings*)GetParam();

	settingsPtr->Set(_T("Calculator:Enable"), (bool)mIsEnableCalc);
	settingsPtr->Set(_T("Soyokaze:PythonDLLPath"), mPythonDLLPath);

	__super::OnOK();
}

void ExtensionSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_ENABLECALCULATOR, mIsEnableCalc);
	DDX_Text(pDX, IDC_EDIT_PYTHONDLLPATH, mPythonDLLPath);
}

BEGIN_MESSAGE_MAP(ExtensionSettingDialog, SettingPage)
	ON_COMMAND(IDC_BUTTON_BROWSE, OnBrowsePyhonDLLPath)
	ON_COMMAND(IDC_CHECK_ENABLECALCULATOR, OnCheckEnableCalculator)
END_MESSAGE_MAP()


BOOL ExtensionSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

BOOL ExtensionSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	if (mIsEnableCalc && PathFileExists(mPythonDLLPath) == FALSE) {
		AfxMessageBox(_T("Python(DLL)のパスを設定してください\n(ファイルが存在しません)"));
		return FALSE;
	}

	return TRUE;
}

BOOL ExtensionSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void ExtensionSettingDialog::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	mIsEnableCalc = settingsPtr->Get(_T("Calculator:Enable"), false);
	mPythonDLLPath = settingsPtr->Get(_T("Soyokaze:PythonDLLPath"), _T(""));
}

bool ExtensionSettingDialog::UpdateStatus()
{
	GetDlgItem(IDC_EDIT_PYTHONDLLPATH)->EnableWindow(mIsEnableCalc);
	GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(mIsEnableCalc);

	return true;
}

void ExtensionSettingDialog::OnBrowsePyhonDLLPath()
{
	UpdateData();

	CString filterStr((LPCTSTR)IDS_FILTER_DLL);
	CFileDialog dlg(TRUE, NULL, mPythonDLLPath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mPythonDLLPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void ExtensionSettingDialog::OnCheckEnableCalculator()
{
	UpdateData();
	UpdateStatus();
}
