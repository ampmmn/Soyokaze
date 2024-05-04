// あ
#include "pch.h"
#include "framework.h"
#include "BasicSettingDialog.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BasicSettingDialog::BasicSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("基本"), IDD_BASICSETTING, parentWnd)
{
}

BasicSettingDialog::~BasicSettingDialog()
{
}

BOOL BasicSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL BasicSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void BasicSettingDialog::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();

	settingsPtr->Set(_T("HotKey:Modifiers"), (int)mHotKeyAttr.GetModifiers());
	settingsPtr->Set(_T("HotKey:VirtualKeyCode"), (int)mHotKeyAttr.GetVKCode());
	settingsPtr->Set(_T("Soyokaze:ShowToggle"), (bool)mIsShowToggle);
	settingsPtr->Set(_T("Soyokaze:IsIKeepTextWhenDlgHide"), (bool)mIsKeepTextWhenDlgHide);
	settingsPtr->Set(_T("Soyokaze:IsHideOnStartup"), (bool)mIsHideOnRun);

	__super::OnOK();
}

void BasicSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_HOTKEY, mHotKey);
	DDX_Check(pDX, IDC_CHECK_SHOWTOGGLE, mIsShowToggle);
	DDX_Check(pDX, IDC_CHECK_KEEPTEXTWHENDLGHIDE, mIsKeepTextWhenDlgHide);
	DDX_Check(pDX, IDC_CHECK_HIDEONRUN, mIsHideOnRun);
}

BEGIN_MESSAGE_MAP(BasicSettingDialog, SettingPage)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
END_MESSAGE_MAP()


BOOL BasicSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool BasicSettingDialog::UpdateStatus()
{
	mHotKey = mHotKeyAttr.ToString();

	return true;
}

void BasicSettingDialog::OnButtonHotKey()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttr, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttr);

	UpdateStatus();
	UpdateData(FALSE);
}

void BasicSettingDialog::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	mHotKeyAttr = HOTKEY_ATTR(settingsPtr->Get(_T("HotKey:Modifiers"), MOD_ALT),
		                        settingsPtr->Get(_T("HotKey:VirtualKeyCode"), VK_SPACE));
	mHotKey = mHotKeyAttr.ToString();

	mIsShowToggle = settingsPtr->Get(_T("Soyokaze:ShowToggle"), true);
	mIsKeepTextWhenDlgHide = settingsPtr->Get(_T("Soyokaze:IsIKeepTextWhenDlgHide"), false);
	mIsHideOnRun = settingsPtr->Get(_T("Soyokaze:IsHideOnStartup"), false);

}

bool BasicSettingDialog::GetHelpPageId(CString& id)
{
	id = _T("GeneralSetting");
	return true;
}

