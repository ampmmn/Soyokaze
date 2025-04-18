#include "pch.h"
#include "framework.h"
#include "InputWindowKeySettingPage.h"
#include "hotkey/HotKeyDialog.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class InputWindowKeySettingPage : public CDialog
{
public:
	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	bool UpdateStatus();

	BOOL OnInitDialog() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonHotKeyUp();
	afx_msg void OnButtonHotKeyDown();
	afx_msg void OnButtonHotKeyEnter();
	afx_msg void OnButtonHotKeyCompl();
	afx_msg void OnButtonHotKeyContextMenu();
	afx_msg void OnButtonHotKeyCopy();
	afx_msg void OnButtonReset();

public:
	// 上へ
	CString mHotKeyUp;
	HOTKEY_ATTR mHotKeyAttrUp;
	// 下へ
	CString mHotKeyDown;
	HOTKEY_ATTR mHotKeyAttrDown;
	// 決定
	CString mHotKeyEnter;
	HOTKEY_ATTR mHotKeyAttrEnter;
	// 補完
	CString mHotKeyCompl;
	HOTKEY_ATTR mHotKeyAttrCompl;
	// コンテキストメニュー表示
	CString mHotKeyContextMenu;
	HOTKEY_ATTR mHotKeyAttrContextMenu;
	// コピー
	CString mHotKeyCopy;
	HOTKEY_ATTR mHotKeyAttrCopy;
	Settings* mSettingsPtr{nullptr};

};


bool InputWindowKeySettingPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool InputWindowKeySettingPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void InputWindowKeySettingPage::OnOK()
{
	auto settingsPtr = mSettingsPtr;
	settingsPtr->Set(_T("MainWindowKey:Up-Modifiers"), (int)mHotKeyAttrUp.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Up-VirtualKeyCode"), (int)mHotKeyAttrUp.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Down-Modifiers"), (int)mHotKeyAttrDown.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Down-VirtualKeyCode"), (int)mHotKeyAttrDown.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Enter-Modifiers"), (int)mHotKeyAttrEnter.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Enter-VirtualKeyCode"), (int)mHotKeyAttrEnter.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Compl-Modifiers"), (int)mHotKeyAttrCompl.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Compl-VirtualKeyCode"), (int)mHotKeyAttrCompl.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:ContextMenu-Modifiers"), (int)mHotKeyAttrContextMenu.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:ContextMenu-VirtualKeyCode"), (int)mHotKeyAttrContextMenu.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Copy-Modifiers"), (int)mHotKeyAttrCopy.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Copy-VirtualKeyCode"), (int)mHotKeyAttrCopy.GetVKCode());
	__super::OnOK();
}

void InputWindowKeySettingPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_HOTKEY_UP, mHotKeyUp);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_DOWN, mHotKeyDown);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_ENTER, mHotKeyEnter);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_COMPL, mHotKeyCompl);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_CONTEXTMENU, mHotKeyContextMenu);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_COPY, mHotKeyCopy);
}

BEGIN_MESSAGE_MAP(InputWindowKeySettingPage, CDialog)
	ON_COMMAND(IDC_BUTTON_HOTKEY_UP, OnButtonHotKeyUp)
	ON_COMMAND(IDC_BUTTON_HOTKEY_DOWN, OnButtonHotKeyDown)
	ON_COMMAND(IDC_BUTTON_HOTKEY_ENTER, OnButtonHotKeyEnter)
	ON_COMMAND(IDC_BUTTON_HOTKEY_COMPL, OnButtonHotKeyCompl)
	ON_COMMAND(IDC_BUTTON_HOTKEY_CONTEXTMENU, OnButtonHotKeyContextMenu)
	ON_COMMAND(IDC_BUTTON_HOTKEY_COPY, OnButtonHotKeyCopy)
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
	mHotKeyContextMenu = mHotKeyAttrContextMenu.ToString();
	mHotKeyCopy = mHotKeyAttrCopy.ToString();
	return true;
}

void InputWindowKeySettingPage::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

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

	mHotKeyAttrContextMenu = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:ContextMenu-Modifiers"), 0),
	                            settingsPtr->Get(_T("MainWindowKey:ContextMenu-VirtualKeyCode"), -1));
	mHotKeyContextMenu = mHotKeyAttrContextMenu.ToString();

	mHotKeyAttrCopy = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:Copy-Modifiers"), 0),
	                            settingsPtr->Get(_T("MainWindowKey:Copy-VirtualKeyCode"), -1));
	mHotKeyCopy = mHotKeyAttrCopy.ToString();
}

void InputWindowKeySettingPage::OnButtonHotKeyUp()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrUp, this);
	dlg.SetTargetName(_T("上へ"));
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
	dlg.SetTargetName(_T("下へ"));
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
	dlg.SetTargetName(_T("決定"));
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
	dlg.SetTargetName(_T("補完"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrCompl);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonHotKeyContextMenu()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrContextMenu, this);
	dlg.SetTargetName(_T("コンテキストメニュー"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrContextMenu);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonHotKeyCopy()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrCopy, this);
	dlg.SetTargetName(_T("コピー"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrCopy);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonReset()
{
	mHotKeyAttrUp = HOTKEY_ATTR();
	mHotKeyAttrDown = HOTKEY_ATTR();
	mHotKeyAttrEnter = HOTKEY_ATTR();
	mHotKeyAttrCompl = HOTKEY_ATTR();
	mHotKeyAttrContextMenu = HOTKEY_ATTR();
	mHotKeyAttrCopy = HOTKEY_ATTR();

	UpdateStatus();
	UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct AppSettingPageInputWindowKey::PImpl
{
	InputWindowKeySettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageInputWindowKey)

AppSettingPageInputWindowKey::AppSettingPageInputWindowKey() : 
	AppSettingPageBase(_T("入力"), _T("キー割り当て")),
	in(new PImpl)
{
}

AppSettingPageInputWindowKey::~AppSettingPageInputWindowKey()
{
}

// ウインドウを作成する
bool AppSettingPageInputWindowKey::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_KEY, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageInputWindowKey::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageInputWindowKey::GetOrder()
{
	return 10;
}
// 
bool AppSettingPageInputWindowKey::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageInputWindowKey::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageInputWindowKey::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageInputWindowKey::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageInputWindowKey::GetHelpPageId(CString& id)
{
	id = _T("InputKeySetting");
	return true;
}

