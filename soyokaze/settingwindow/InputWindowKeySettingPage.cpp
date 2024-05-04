#include "pch.h"
#include "framework.h"
#include "InputWindowKeySettingPage.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

InputWindowKeySettingPage::InputWindowKeySettingPage(CWnd* parentWnd) : 
	SettingPage(_T("キー割り当て"), IDD_APPSETTING_KEY, parentWnd)
{
}

InputWindowKeySettingPage::~InputWindowKeySettingPage()
{
}

BOOL InputWindowKeySettingPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL InputWindowKeySettingPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void InputWindowKeySettingPage::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("MainWindowKey:Up-Modifiers"), (int)mHotKeyAttrUp.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Up-VirtualKeyCode"), (int)mHotKeyAttrUp.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Down-Modifiers"), (int)mHotKeyAttrDown.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Down-VirtualKeyCode"), (int)mHotKeyAttrDown.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Enter-Modifiers"), (int)mHotKeyAttrEnter.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Enter-VirtualKeyCode"), (int)mHotKeyAttrEnter.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Compl-Modifiers"), (int)mHotKeyAttrCompl.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Compl-VirtualKeyCode"), (int)mHotKeyAttrCompl.GetVKCode());
	__super::OnOK();
}

void InputWindowKeySettingPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_HOTKEY_UP, mHotKeyUp);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_DOWN, mHotKeyDown);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_ENTER, mHotKeyEnter);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_COMPL, mHotKeyCompl);
}

BEGIN_MESSAGE_MAP(InputWindowKeySettingPage, SettingPage)
	ON_COMMAND(IDC_BUTTON_HOTKEY_UP, OnButtonHotKeyUp)
	ON_COMMAND(IDC_BUTTON_HOTKEY_DOWN, OnButtonHotKeyDown)
	ON_COMMAND(IDC_BUTTON_HOTKEY_ENTER, OnButtonHotKeyEnter)
	ON_COMMAND(IDC_BUTTON_HOTKEY_COMPL, OnButtonHotKeyCompl)
	ON_COMMAND(IDC_BUTTON_RESET, OnButtonReset)
END_MESSAGE_MAP()


BOOL InputWindowKeySettingPage::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool InputWindowKeySettingPage::UpdateStatus()
{
	mHotKeyUp = mHotKeyAttrUp.ToString();
	mHotKeyDown = mHotKeyAttrDown.ToString();
	mHotKeyEnter = mHotKeyAttrEnter.ToString();
	mHotKeyCompl = mHotKeyAttrCompl.ToString();
	return true;
}

void InputWindowKeySettingPage::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	mHotKeyAttrUp = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:Up-Modifiers"), 0),
	                            settingsPtr->Get(_T("MainWindowKey:Up-VirtualKeyCode"), -1));
	mHotKeyUp = mHotKeyAttrUp.ToString();

	mHotKeyAttrDown = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:Down-Modifiers"), 0),
	                            settingsPtr->Get(_T("MainWindowKey:Down-VirtualKeyCode"), -1));
	mHotKeyDown = mHotKeyAttrDown.ToString();

	mHotKeyAttrEnter = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:Enter-Modifiers"), 0),
	                            settingsPtr->Get(_T("MainWindowKey:Enter-VirtualKeyCode"), -1));
	mHotKeyEnter = mHotKeyAttrEnter.ToString();

	mHotKeyAttrCompl = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:Compl-Modifiers"), 0),
	                            settingsPtr->Get(_T("MainWindowKey:Compl-VirtualKeyCode"), -1));
	mHotKeyCompl = mHotKeyAttrCompl.ToString();
}

bool InputWindowKeySettingPage::GetHelpPageId(CString& id)
{
	id = _T("InputKeySetting");
	return true;
}


void InputWindowKeySettingPage::OnButtonHotKeyUp()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrUp, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrUp);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonHotKeyDown()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrDown, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrDown);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonHotKeyEnter()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrEnter, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrEnter);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonHotKeyCompl()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrCompl, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrCompl);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonReset()
{
	mHotKeyAttrUp = HOTKEY_ATTR();
	mHotKeyAttrDown = HOTKEY_ATTR();
	mHotKeyAttrEnter = HOTKEY_ATTR();
	mHotKeyAttrCompl = HOTKEY_ATTR();

	UpdateStatus();
	UpdateData(FALSE);
}


