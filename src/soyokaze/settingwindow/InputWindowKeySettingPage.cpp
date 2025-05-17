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
	afx_msg void OnButtonHotKeySelectUp();
	afx_msg void OnButtonHotKeySelectDown();
	afx_msg void OnButtonHotKeyEnter();
	afx_msg void OnButtonHotKeyCompl();
	afx_msg void OnButtonHotKeyContextMenu();
	afx_msg void OnButtonHotKeyCopy();
	afx_msg void OnButtonHotKeyMoveUp();
	afx_msg void OnButtonHotKeyMoveDown();
	afx_msg void OnButtonHotKeyMoveLeft();
	afx_msg void OnButtonHotKeyMoveRight();
	afx_msg void OnButtonReset();

public:
	// 選択を上へ
	CString mHotKeySelectUp;
	HOTKEY_ATTR mHotKeyAttrSelectUp;
	// 選択を下へ
	CString mHotKeySelectDown;
	HOTKEY_ATTR mHotKeyAttrSelectDown;
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
	// ウインドウを上端へ移動
	CString mHotKeyMoveUp;
	HOTKEY_ATTR mHotKeyAttrMoveUp;
	// ウインドウを下端へ移動
	CString mHotKeyMoveDown;
	HOTKEY_ATTR mHotKeyAttrMoveDown;
	// ウインドウを左端へ移動
	CString mHotKeyMoveLeft;
	HOTKEY_ATTR mHotKeyAttrMoveLeft;
	// ウインドウを右端へ移動
	CString mHotKeyMoveRight;
	HOTKEY_ATTR mHotKeyAttrMoveRight;

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
	settingsPtr->Set(_T("MainWindowKey:Up-Modifiers"), (int)mHotKeyAttrSelectUp.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Up-VirtualKeyCode"), (int)mHotKeyAttrSelectUp.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Down-Modifiers"), (int)mHotKeyAttrSelectDown.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Down-VirtualKeyCode"), (int)mHotKeyAttrSelectDown.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Enter-Modifiers"), (int)mHotKeyAttrEnter.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Enter-VirtualKeyCode"), (int)mHotKeyAttrEnter.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Compl-Modifiers"), (int)mHotKeyAttrCompl.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Compl-VirtualKeyCode"), (int)mHotKeyAttrCompl.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:ContextMenu-Modifiers"), (int)mHotKeyAttrContextMenu.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:ContextMenu-VirtualKeyCode"), (int)mHotKeyAttrContextMenu.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:Copy-Modifiers"), (int)mHotKeyAttrCopy.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:Copy-VirtualKeyCode"), (int)mHotKeyAttrCopy.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:MoveUp-Modifiers"), (int)mHotKeyAttrMoveUp.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:MoveUp-VirtualKeyCode"), (int)mHotKeyAttrMoveUp.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:MoveDown-Modifiers"), (int)mHotKeyAttrMoveDown.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:MoveDown-VirtualKeyCode"), (int)mHotKeyAttrMoveDown.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:MoveLeft-Modifiers"), (int)mHotKeyAttrMoveLeft.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:MoveLeft-VirtualKeyCode"), (int)mHotKeyAttrMoveLeft.GetVKCode());

	settingsPtr->Set(_T("MainWindowKey:MoveRight-Modifiers"), (int)mHotKeyAttrMoveRight.GetModifiers());
	settingsPtr->Set(_T("MainWindowKey:MoveRight-VirtualKeyCode"), (int)mHotKeyAttrMoveRight.GetVKCode());

	__super::OnOK();
}

void InputWindowKeySettingPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_HOTKEY_UP, mHotKeySelectUp);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_DOWN, mHotKeySelectDown);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_ENTER, mHotKeyEnter);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_COMPL, mHotKeyCompl);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_CONTEXTMENU, mHotKeyContextMenu);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_COPY, mHotKeyCopy);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_MOVE_UP, mHotKeyMoveUp);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_MOVE_DOWN, mHotKeyMoveDown);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_MOVE_LEFT, mHotKeyMoveLeft);
	DDX_Text(pDX, IDC_EDIT_HOTKEY_MOVE_RIGHT, mHotKeyMoveRight);
}

BEGIN_MESSAGE_MAP(InputWindowKeySettingPage, CDialog)
	ON_COMMAND(IDC_BUTTON_HOTKEY_UP, OnButtonHotKeySelectUp)
	ON_COMMAND(IDC_BUTTON_HOTKEY_DOWN, OnButtonHotKeySelectDown)
	ON_COMMAND(IDC_BUTTON_HOTKEY_ENTER, OnButtonHotKeyEnter)
	ON_COMMAND(IDC_BUTTON_HOTKEY_COMPL, OnButtonHotKeyCompl)
	ON_COMMAND(IDC_BUTTON_HOTKEY_CONTEXTMENU, OnButtonHotKeyContextMenu)
	ON_COMMAND(IDC_BUTTON_HOTKEY_COPY, OnButtonHotKeyCopy)
	ON_COMMAND(IDC_BUTTON_HOTKEY_MOVE_UP, OnButtonHotKeyMoveUp)
	ON_COMMAND(IDC_BUTTON_HOTKEY_MOVE_DOWN, OnButtonHotKeyMoveDown)
	ON_COMMAND(IDC_BUTTON_HOTKEY_MOVE_LEFT, OnButtonHotKeyMoveLeft)
	ON_COMMAND(IDC_BUTTON_HOTKEY_MOVE_RIGHT, OnButtonHotKeyMoveRight)
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
	mHotKeySelectUp = mHotKeyAttrSelectUp.ToString();
	mHotKeySelectDown = mHotKeyAttrSelectDown.ToString();
	mHotKeyEnter = mHotKeyAttrEnter.ToString();
	mHotKeyCompl = mHotKeyAttrCompl.ToString();
	mHotKeyContextMenu = mHotKeyAttrContextMenu.ToString();
	mHotKeyCopy = mHotKeyAttrCopy.ToString();
	mHotKeyMoveUp = mHotKeyAttrMoveUp.ToString();
	mHotKeyMoveDown = mHotKeyAttrMoveDown.ToString();
	mHotKeyMoveLeft = mHotKeyAttrMoveLeft.ToString();
	mHotKeyMoveRight = mHotKeyAttrMoveRight.ToString();
	return true;
}

void InputWindowKeySettingPage::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

	mHotKeyAttrSelectUp = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:Up-Modifiers"), 0),
	                                  settingsPtr->Get(_T("MainWindowKey:Up-VirtualKeyCode"), -1));
	mHotKeySelectUp = mHotKeyAttrSelectUp.ToString();

	mHotKeyAttrSelectDown = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:Down-Modifiers"), 0),
	                                    settingsPtr->Get(_T("MainWindowKey:Down-VirtualKeyCode"), -1));
	mHotKeySelectDown = mHotKeyAttrSelectDown.ToString();

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

	mHotKeyAttrMoveUp = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:MoveUp-Modifiers"), 0),
	                                settingsPtr->Get(_T("MainWindowKey:MoveUp-VirtualKeyCode"), -1));
	mHotKeyMoveUp = mHotKeyAttrMoveUp.ToString();

	mHotKeyAttrMoveDown = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:MoveDown-Modifiers"), 0),
	                                  settingsPtr->Get(_T("MainWindowKey:MoveDown-VirtualKeyCode"), -1));
	mHotKeyMoveDown = mHotKeyAttrMoveDown.ToString();

	mHotKeyAttrMoveLeft = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:MoveLeft-Modifiers"), 0),
	                                  settingsPtr->Get(_T("MainWindowKey:MoveLeft-VirtualKeyCode"), -1));
	mHotKeyMoveLeft = mHotKeyAttrMoveLeft.ToString();

	mHotKeyAttrMoveRight = HOTKEY_ATTR(settingsPtr->Get(_T("MainWindowKey:MoveRight-Modifiers"), 0),
	                                   settingsPtr->Get(_T("MainWindowKey:MoveRight-VirtualKeyCode"), -1));
	mHotKeyMoveRight = mHotKeyAttrMoveRight.ToString();
}

void InputWindowKeySettingPage::OnButtonHotKeySelectUp()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrSelectUp, this);
	dlg.SetTargetName(_T("上へ"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrSelectUp);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonHotKeySelectDown()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrSelectDown, this);
	dlg.SetTargetName(_T("下へ"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrSelectDown);

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

void InputWindowKeySettingPage::OnButtonHotKeyMoveUp()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrMoveUp, this);
	dlg.SetTargetName(_T("上端に移動"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrMoveUp);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonHotKeyMoveDown()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrMoveDown, this);
	dlg.SetTargetName(_T("下端に移動"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrMoveDown);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonHotKeyMoveLeft()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrMoveLeft, this);
	dlg.SetTargetName(_T("左端に移動"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrMoveLeft);

	UpdateStatus();
	UpdateData(FALSE);
}

void InputWindowKeySettingPage::OnButtonHotKeyMoveRight()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttrMoveRight, this);
	dlg.SetTargetName(_T("右端に移動"));
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttrMoveRight);

	UpdateStatus();
	UpdateData(FALSE);
}


void InputWindowKeySettingPage::OnButtonReset()
{
	mHotKeyAttrSelectUp = HOTKEY_ATTR();
	mHotKeyAttrSelectDown = HOTKEY_ATTR();
	mHotKeyAttrEnter = HOTKEY_ATTR();
	mHotKeyAttrCompl = HOTKEY_ATTR();
	mHotKeyAttrContextMenu = HOTKEY_ATTR();
	mHotKeyAttrCopy = HOTKEY_ATTR();
	mHotKeyAttrMoveUp = HOTKEY_ATTR();
	mHotKeyAttrMoveDown = HOTKEY_ATTR();
	mHotKeyAttrMoveLeft = HOTKEY_ATTR();
	mHotKeyAttrMoveRight = HOTKEY_ATTR();

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

