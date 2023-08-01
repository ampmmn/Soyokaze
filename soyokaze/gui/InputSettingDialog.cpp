#include "pch.h"
#include "framework.h"
#include "InputSettingDialog.h"
#include "Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


InputSettingDialog::InputSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("入力"), IDD_INPUTSETTING, parentWnd)
{
}

InputSettingDialog::~InputSettingDialog()
{
}

BOOL InputSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL InputSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void InputSettingDialog::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("Soyokaze:IsIMEOffOnActive"), (bool)mIsIMEOff);
	settingsPtr->Set(_T("Soyokaze:IsIgnoreUNC"), (bool)mIsIgnoreUNC);
	settingsPtr->Set(_T("Soyokaze:IsEnableMigemo"), (bool)mIsEnableMigemo);

	__super::OnOK();
}

void InputSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_IMEOFF, mIsIMEOff);
	DDX_Check(pDX, IDC_CHECK_IGNOREUNC, mIsIgnoreUNC);
	DDX_Check(pDX, IDC_CHECK_ENABLEMIGEMO, mIsEnableMigemo);
}

BEGIN_MESSAGE_MAP(InputSettingDialog, SettingPage)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, OnNotifyLinkOpen)
END_MESSAGE_MAP()


BOOL InputSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool InputSettingDialog::UpdateStatus()
{
	return true;
}

void InputSettingDialog::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();
	mIsIMEOff = settingsPtr->Get(_T("Soyokaze:IsIMEOffOnActive"), false);
	mIsIgnoreUNC = settingsPtr->Get(_T("Soyokaze:IsIgnoreUNC"), false);
	mIsEnableMigemo = settingsPtr->Get(_T("Soyokaze:IsEnableMigemo"), true);
}

void InputSettingDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	NMLINK* linkPtr = (NMLINK*)pNMHDR;

	ShellExecute(0, _T("open"), linkPtr->item.szUrl,  0, 0,SW_NORMAL);
	*pResult = 0;
}

