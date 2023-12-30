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
	mIsEnableCalc(FALSE),
	mIsEnableWindowTitle(FALSE),
	mIsEnableWorksheet(FALSE),
	mIsEnableControlPanel(FALSE),
	mIsEnableSpecialFolder(FALSE),
	mIsEnableUWP(FALSE),
	mIsEnableOutlookMail(FALSE)
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
	settingsPtr->Set(_T("WindowSwitch:EnableWindowSwitch"), (bool)mIsEnableWindowTitle);
	settingsPtr->Set(_T("Excel:EnableWorkSheet"), (bool)mIsEnableWorksheet);
	settingsPtr->Set(_T("Soyokaze:IsEnableControlPanel"), (bool)mIsEnableControlPanel);
	settingsPtr->Set(_T("Soyokaze:IsEnableSpecialFolder"), (bool)mIsEnableSpecialFolder);
	settingsPtr->Set(_T("Soyokaze:IsEnableUWP"), (bool)mIsEnableUWP);
	settingsPtr->Set(_T("Soyokaze:IsEnableOutlookMailItem"), (bool)mIsEnableOutlookMail);
	__super::OnOK();
}

void ExtensionSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_ENABLECALCULATOR, mIsEnableCalc);
	DDX_Text(pDX, IDC_EDIT_PYTHONDLLPATH, mPythonDLLPath);
	DDX_Check(pDX, IDC_CHECK_ENABLE_WINDOWTITLE, mIsEnableWindowTitle);
	DDX_Check(pDX, IDC_CHECK_ENABLE_WORKSHEET, mIsEnableWorksheet);
	DDX_Check(pDX, IDC_CHECK_ENABLE_CONTROLPANEL, mIsEnableControlPanel);
	DDX_Check(pDX, IDC_CHECK_ENABLE_SPECIALFOLDER, mIsEnableSpecialFolder);
	DDX_Check(pDX, IDC_CHECK_ENABLE_UWPAPPS, mIsEnableUWP);
	DDX_Check(pDX, IDC_CHECK_ENABLE_OUTLOOKMAIL, mIsEnableOutlookMail);
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

	mIsEnableWindowTitle = settingsPtr->Get(_T("WindowSwitch:EnableWindowSwitch"), true);
	mIsEnableWorksheet = settingsPtr->Get(_T("Excel:EnableWorkSheet"), true);
	mIsEnableControlPanel = settingsPtr->Get(_T("Soyokaze:IsEnableControlPanel"), true);
	mIsEnableSpecialFolder = settingsPtr->Get(_T("Soyokaze:IsEnableSpecialFolder"), true);
	mIsEnableUWP = settingsPtr->Get(_T("Soyokaze:IsEnableUWP"), true);
	mIsEnableOutlookMail = settingsPtr->Get(_T("Soyokaze:IsEnableOutlookMailItem"), false);
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
