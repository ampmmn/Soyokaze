#include "pch.h"
#include "framework.h"
#include "InputSettingDialog.h"
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
	mSettingsPtr->Set(_T("Soyokaze:MatchLevel"), mMatchLevel);
	mSettingsPtr->Set(_T("Soyokaze:IsIMEOffOnActive"), (bool)mIsIMEOff);
	mSettingsPtr->Set(_T("Soyokaze:IsIgnoreUNC"), (bool)mIsIgnoreUNC);

	__super::OnOK();
}

void InputSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_CBIndex(pDX, IDC_COMBO_MATCHLEVEL, mMatchLevel);
	DDX_Check(pDX, IDC_CHECK_IMEOFF, mIsIMEOff);
	DDX_Check(pDX, IDC_CHECK_IGNOREUNC, mIsIgnoreUNC);
}

BEGIN_MESSAGE_MAP(InputSettingDialog, SettingPage)
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
	mMatchLevel = mSettingsPtr->Get(_T("Soyokaze:MatchLevel"), 1);
	mIsIMEOff = mSettingsPtr->Get(_T("Soyokaze:IsIMEOffOnActive"), false);
	mIsIgnoreUNC = mSettingsPtr->Get(_T("Soyokaze:IsIgnoreUNC"), false);

}
