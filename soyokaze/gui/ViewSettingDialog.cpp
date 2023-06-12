#include "pch.h"
#include "framework.h"
#include "ViewSettingDialog.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


ViewSettingDialog::ViewSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("表示"), IDD_VIEWSETTING, parentWnd),
	mAlpha(128)
{
}

ViewSettingDialog::~ViewSettingDialog()
{
}

BOOL ViewSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL ViewSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void ViewSettingDialog::OnOK()
{
	mSettingsPtr->Set(_T("Soyokaze:TopMost"), (bool)mIsTopMost);
	mSettingsPtr->Set(_T("Soyokaze:IsHideOnInactive"), (bool)mIsHideOnInactive);

	if (mTransparencyType == 0) {
		mSettingsPtr->Set(_T("WindowTransparency:Enable"), true);
		mSettingsPtr->Set(_T("WindowTransparency:InactiveOnly"), true);
	}
	else if (mTransparencyType == 1) {
		mSettingsPtr->Set(_T("WindowTransparency:Enable"), true);
		mSettingsPtr->Set(_T("WindowTransparency:InactiveOnly"), false);
	}
	else {
		mSettingsPtr->Set(_T("WindowTransparency:Enable"), false);
		mSettingsPtr->Set(_T("WindowTransparency:InactiveOnly"), true);
	}

	mSettingsPtr->Set(_T("WindowTransparency:Alpha"), (int)mAlpha);

	__super::OnOK();
}

void ViewSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_TOPMOST, mIsTopMost);
	DDX_Check(pDX, IDC_CHECK_HIDEONINACTIVE, mIsHideOnInactive);
	DDX_CBIndex(pDX, IDC_COMBO_TRANSPARENCY, mTransparencyType);
	DDX_Text(pDX, IDC_EDIT_ALPHA, mAlpha);
	DDV_MinMaxInt(pDX, mAlpha, 0, 255);
}

BEGIN_MESSAGE_MAP(ViewSettingDialog, SettingPage)
	ON_CBN_SELCHANGE(IDC_COMBO_TRANSPARENCY, OnCbnTransparencyChanged)
END_MESSAGE_MAP()


BOOL ViewSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ViewSettingDialog::UpdateStatus()
{
	GetDlgItem(IDC_EDIT_ALPHA)->EnableWindow(mTransparencyType != 2);

	return true;
}


void ViewSettingDialog::OnCbnTransparencyChanged()
{
	UpdateData();
	UpdateStatus();
}

void ViewSettingDialog::OnEnterSettings()
{
	mIsTopMost = mSettingsPtr->Get(_T("Soyokaze:TopMost"), false);
	mIsHideOnInactive = mSettingsPtr->Get(_T("Soyokaze:IsHideOnInactive"), false);

	if (mSettingsPtr->Get(_T("WindowTransparency:Enable"), false) == false) {
		mTransparencyType = 2;
	}
	else if (mSettingsPtr->Get(_T("WindowTransparency:InactiveOnly"), true)) {
		mTransparencyType = 0;
	}
	else {
		mTransparencyType = 1;
	}

	mAlpha = mSettingsPtr->Get(_T("WindowTransparency:Alpha"), 128);
	if (mAlpha < 0) { mAlpha = 0; }
	if (mAlpha > 255) { mAlpha = 255; }

}
