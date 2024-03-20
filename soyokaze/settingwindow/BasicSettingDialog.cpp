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
	SettingPage(_LANG_T("General"), IDD_BASICSETTING, parentWnd)
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

	if (0 <= mLanguage && mLanguage < (int)mLangCodes.size()) {
		settingsPtr->Set(_T("Soyokaze:Language"), mLangCodes[mLanguage].mCode);
	}

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
	DDX_CBIndex(pDX, IDC_COMBO_LANGUAGE, mLanguage);
}

BEGIN_MESSAGE_MAP(BasicSettingDialog, SettingPage)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
END_MESSAGE_MAP()


BOOL BasicSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	_LANG_WINDOW(GetSafeHwnd());

	soyokaze::core::Honyaku::Get()->EnumLangCodes(mLangCodes);

	auto comboLang = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
	ASSERT(comboLang);

	for (auto& langCode: mLangCodes) {
		comboLang->AddString(langCode.mDisplayName);
	}

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

	auto lang = settingsPtr->Get(_T("Soyokaze:Language"), GetACP() == 932 ? _T("ja") : _T("en"));

	for (size_t i = 0; i < mLangCodes.size(); ++i) {
		if (lang != mLangCodes[i].mCode) {
			continue;
		}
		auto comboLang = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
		if (comboLang) {
			comboLang->SetCurSel((int)i);
		}
		break;
	}

	mHotKeyAttr = HOTKEY_ATTR(settingsPtr->Get(_T("HotKey:Modifiers"), MOD_ALT),
		                        settingsPtr->Get(_T("HotKey:VirtualKeyCode"), VK_SPACE));
	mHotKey = mHotKeyAttr.ToString();

	mIsShowToggle = settingsPtr->Get(_T("Soyokaze:ShowToggle"), true);
	mIsKeepTextWhenDlgHide = settingsPtr->Get(_T("Soyokaze:IsIKeepTextWhenDlgHide"), false);
	mIsHideOnRun = settingsPtr->Get(_T("Soyokaze:IsHideOnStartup"), false);

}
