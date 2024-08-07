// あ
#include "pch.h"
#include "framework.h"
#include "BasicSettingDialog.h"
#include "hotkey/AppHotKeyDialog.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BasicSettingDialog::BasicSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("基本"), IDD_BASICSETTING, parentWnd),
	mIsEnableHotKey(true),
	mIsEnableModifierHotKey(false),
	mModifierFirstVK(VK_CONTROL),
	mModifierSecondVK(VK_CONTROL),
	mIsShowToggle(true),
	mIsKeepTextWhenDlgHide(false),
	mIsHideOnRun(false)
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
	settingsPtr->Set(_T("HotKey:IsEnableHotKey"), mIsEnableHotKey);
	settingsPtr->Set(_T("HotKey:IsEnableModifierHotKey"), mIsEnableModifierHotKey);
	settingsPtr->Set(_T("HotKey:FirstModifierVirtualKeyCode"), (int)mModifierFirstVK);
	settingsPtr->Set(_T("HotKey:SecondModifierVirtualKeyCode"),(int) mModifierSecondVK);

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
	CString text;
	if (mIsEnableHotKey) {
		text = mHotKeyAttr.ToString();
	}
	if (mIsEnableModifierHotKey) {
		if (text.IsEmpty() == FALSE) {
			text += _T(" / ");
		}
		text += AppHotKeyDialog::ToString(mModifierFirstVK, mModifierSecondVK);
	}
	mHotKey = text;

	return true;
}

void BasicSettingDialog::OnButtonHotKey()
{
	UpdateData();

	AppHotKeyDialog dlg(mHotKeyAttr, this);
	dlg.SetTargetName(_T("ランチャー呼び出しキー"));
	dlg.SetEnableHotKey(mIsEnableHotKey);
	dlg.SetEnableModifierHotKey(mIsEnableModifierHotKey);
	dlg.SetModifierFirstVK(mModifierFirstVK);
	dlg.SetModifierSecondVK(mModifierSecondVK);

	if (dlg.DoModal() != IDOK) {
		return ;
	}
	dlg.GetAttribute(mHotKeyAttr);
	mIsEnableHotKey = dlg.IsEnableHotKey();
	mIsEnableModifierHotKey = dlg.IsEnableModifierHotKey();
	mModifierFirstVK = dlg.GetModifierFirstVK();
	mModifierSecondVK = dlg.GetModifierSecondVK();

	UpdateStatus();
	UpdateData(FALSE);
}

void BasicSettingDialog::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	mHotKeyAttr = HOTKEY_ATTR(settingsPtr->Get(_T("HotKey:Modifiers"), MOD_ALT),
		                        settingsPtr->Get(_T("HotKey:VirtualKeyCode"), VK_SPACE));
	mHotKey = mHotKeyAttr.ToString();

	mIsEnableHotKey = settingsPtr->Get(_T("HotKey:IsEnableHotKey"), true);
	mIsEnableModifierHotKey = settingsPtr->Get(_T("HotKey:IsEnableModifierHotKey"), false);
	mModifierFirstVK = settingsPtr->Get(_T("HotKey:FirstModifierVirtualKeyCode"), VK_CONTROL);
	mModifierSecondVK = settingsPtr->Get(_T("HotKey:SecondModifierVirtualKeyCode"), VK_CONTROL);

	mIsShowToggle = settingsPtr->Get(_T("Soyokaze:ShowToggle"), true);
	mIsKeepTextWhenDlgHide = settingsPtr->Get(_T("Soyokaze:IsIKeepTextWhenDlgHide"), false);
	mIsHideOnRun = settingsPtr->Get(_T("Soyokaze:IsHideOnStartup"), false);

}

bool BasicSettingDialog::GetHelpPageId(CString& id)
{
	id = _T("GeneralSetting");
	return true;
}

