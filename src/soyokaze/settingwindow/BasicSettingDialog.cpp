// あ
#include "pch.h"
#include "framework.h"
#include "BasicSettingDialog.h"
#include "hotkey/HotKeyAttribute.h"
#include "hotkey/AppHotKeyDialog.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum POSITIONTYPE {
	POSTYPE_KEEPLAST = 0,        // 前回と同じ位置
	POSTYPE_MOUSECURSOR,         // マウスカーソル
	POSTYPE_ACTIVEWINDOWCENTER,  // アクティブなウインドウの中央
};


class BasicSettingDialog : public CDialog
{
public:
	BasicSettingDialog();
	virtual ~BasicSettingDialog();

	bool UpdateStatus();

	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

	virtual void OnOK();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonHotKey();

private:
	CString mHotKey;
	HOTKEY_ATTR mHotKeyAttr;

	bool mIsEnableHotKey{true};
	bool mIsEnableModifierHotKey{false};
	bool mIsEnableModifierHotKeyOnRD{false};
	UINT mModifierFirstVK{VK_CONTROL};
	UINT mModifierSecondVK{VK_CONTROL};

	// 入力画面の表示位置
	int mShowPositionType{POSTYPE_KEEPLAST};

	// 表示中にホットキーを押したら隠れる
	BOOL mIsShowToggle{true};
	// 入力画面を非表示にするときに入力文字列を消去しない
	BOOL mIsKeepTextWhenDlgHide{false};
	// 起動直後は入力画面を非表示にする
	BOOL mIsHideOnRun{false};
	// 入力画面を常に最前面に表示
	BOOL mIsTopMost{FALSE};
	// アクティブ状態でなくなったらウインドウを隠す
	BOOL mIsHideOnInactive{FALSE};
	// マウスカーソル位置に入力欄を表示する
	BOOL mIsShowMainWindowOnCursor{FALSE};

	Settings* mSettingsPtr{nullptr};
};



BasicSettingDialog::BasicSettingDialog()
{
}

BasicSettingDialog::~BasicSettingDialog()
{
}

void BasicSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	mHotKeyAttr = HOTKEY_ATTR(settingsPtr->Get(_T("HotKey:Modifiers"), MOD_ALT),
		                        settingsPtr->Get(_T("HotKey:VirtualKeyCode"), VK_SPACE));
	mHotKey = mHotKeyAttr.ToString();

	mIsEnableHotKey = settingsPtr->Get(_T("HotKey:IsEnableHotKey"), true);
	mIsEnableModifierHotKey = settingsPtr->Get(_T("HotKey:IsEnableModifierHotKey"), false);
	mIsEnableModifierHotKeyOnRD = settingsPtr->Get(_T("HotKey:IsEnableModifierHotKeyOnRD"), false);
	mModifierFirstVK = settingsPtr->Get(_T("HotKey:FirstModifierVirtualKeyCode"), VK_CONTROL);
	mModifierSecondVK = settingsPtr->Get(_T("HotKey:SecondModifierVirtualKeyCode"), VK_CONTROL);

	mIsShowToggle = settingsPtr->Get(_T("Soyokaze:ShowToggle"), true);
	mIsKeepTextWhenDlgHide = settingsPtr->Get(_T("Soyokaze:IsIKeepTextWhenDlgHide"), false);
	mIsHideOnRun = settingsPtr->Get(_T("Soyokaze:IsHideOnStartup"), false);
	mIsTopMost = settingsPtr->Get(_T("Soyokaze:TopMost"), false);
	mIsHideOnInactive = settingsPtr->Get(_T("Soyokaze:IsHideOnInactive"), false);

	bool isShowOnCursor = settingsPtr->Get(_T("Soyokaze:IsShowMainWindowOnCurorPos"), false);
	bool isShowOnActWin = settingsPtr->Get(_T("Soyokaze:IsShowMainWindowOnActiveWindowCenter"), false);
	if (isShowOnCursor) {
		mShowPositionType = POSTYPE_MOUSECURSOR;
	}
	else if (isShowOnActWin) {
		mShowPositionType = POSTYPE_ACTIVEWINDOWCENTER;
	}
	else {
		mShowPositionType = POSTYPE_KEEPLAST;
	}
}

bool BasicSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

bool BasicSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

void BasicSettingDialog::OnOK()
{
	auto settingsPtr = mSettingsPtr;

	settingsPtr->Set(_T("HotKey:Modifiers"), (int)mHotKeyAttr.GetModifiers());
	settingsPtr->Set(_T("HotKey:VirtualKeyCode"), (int)mHotKeyAttr.GetVKCode());
	settingsPtr->Set(_T("HotKey:IsEnableHotKey"), mIsEnableHotKey);
	settingsPtr->Set(_T("HotKey:IsEnableModifierHotKey"), mIsEnableModifierHotKey);
	settingsPtr->Set(_T("HotKey:IsEnableModifierHotKeyOnRD"), mIsEnableModifierHotKeyOnRD);
	settingsPtr->Set(_T("HotKey:FirstModifierVirtualKeyCode"), (int)mModifierFirstVK);
	settingsPtr->Set(_T("HotKey:SecondModifierVirtualKeyCode"),(int) mModifierSecondVK);

	settingsPtr->Set(_T("Soyokaze:ShowToggle"), (bool)mIsShowToggle);
	settingsPtr->Set(_T("Soyokaze:IsIKeepTextWhenDlgHide"), (bool)mIsKeepTextWhenDlgHide);
	settingsPtr->Set(_T("Soyokaze:IsHideOnStartup"), (bool)mIsHideOnRun);
	settingsPtr->Set(_T("Soyokaze:TopMost"), (bool)mIsTopMost);
	settingsPtr->Set(_T("Soyokaze:IsHideOnInactive"), (bool)mIsHideOnInactive);

	bool isShowOnActWin = (mShowPositionType == POSTYPE_MOUSECURSOR);
	settingsPtr->Set(_T("Soyokaze:IsShowMainWindowOnCurorPos"), isShowOnActWin);
	bool isShowOnCursor = (mShowPositionType == POSTYPE_ACTIVEWINDOWCENTER);
	settingsPtr->Set(_T("Soyokaze:IsShowMainWindowOnActiveWindowCenter"), isShowOnCursor);

	CDialog::OnOK();
}

void BasicSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_HOTKEY, mHotKey);
	DDX_Check(pDX, IDC_CHECK_SHOWTOGGLE, mIsShowToggle);
	DDX_Check(pDX, IDC_CHECK_KEEPTEXTWHENDLGHIDE, mIsKeepTextWhenDlgHide);
	DDX_Check(pDX, IDC_CHECK_HIDEONRUN, mIsHideOnRun);
	DDX_Check(pDX, IDC_CHECK_TOPMOST, mIsTopMost);
	DDX_Check(pDX, IDC_CHECK_HIDEONINACTIVE, mIsHideOnInactive);
	DDX_CBIndex(pDX, IDC_COMBO_POSITION, mShowPositionType);
}

BEGIN_MESSAGE_MAP(BasicSettingDialog, CDialog)
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
	dlg.SetEnableHotKey(mIsEnableHotKey);
	dlg.SetEnableModifierHotKey(mIsEnableModifierHotKey);
	dlg.SetEnableModifierHotKeyOnRD(mIsEnableModifierHotKeyOnRD);
	dlg.SetModifierFirstVK(mModifierFirstVK);
	dlg.SetModifierSecondVK(mModifierSecondVK);

	if (dlg.DoModal() != IDOK) {
		return ;
	}
	dlg.GetAttribute(mHotKeyAttr);
	mIsEnableHotKey = dlg.IsEnableHotKey();
	mIsEnableModifierHotKey = dlg.IsEnableModifierHotKey();
	mIsEnableModifierHotKeyOnRD = dlg.IsEnableModifierHotKeyOnRD();
	mModifierFirstVK = dlg.GetModifierFirstVK();
	mModifierSecondVK = dlg.GetModifierSecondVK();

	UpdateStatus();
	UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageBasic::PImpl
{
	BasicSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageBasic)

AppSettingPageBasic::AppSettingPageBasic() : 
	AppSettingPageBase(_T(""), _T("基本")),
	in(new PImpl)
{
}

AppSettingPageBasic::~AppSettingPageBasic()
{
}

// ウインドウを作成する
bool AppSettingPageBasic::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_BASICSETTING, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageBasic::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageBasic::GetOrder()
{
	return 10;
}
// 
bool AppSettingPageBasic::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageBasic::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageBasic::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageBasic::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageBasic::GetHelpPageId(String& id)
{
	id = "GeneralSetting";
	return true;
}

