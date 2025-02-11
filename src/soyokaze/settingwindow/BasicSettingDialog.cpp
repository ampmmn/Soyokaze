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


struct BasicSettingDialog::PImpl
{
	// ランチャー呼び出しキー（表示用)
	CString mHotKey;
	HOTKEY_ATTR mHotKeyAttr;

	bool mIsEnableHotKey = true;
	bool mIsEnableModifierHotKey = false;
	bool mIsEnableModifierHotKeyOnRD = false;
	UINT mModifierFirstVK = VK_CONTROL;
	UINT mModifierSecondVK = VK_CONTROL;

	// 入力画面の表示位置
	int mShowPositionType = POSTYPE_KEEPLAST;

	// 表示中にホットキーを押したら隠れる
	BOOL mIsShowToggle = true;
	// 入力画面を非表示にするときに入力文字列を消去しない
	BOOL mIsKeepTextWhenDlgHide = false;
	// 起動直後は入力画面を非表示にする
	BOOL mIsHideOnRun = false;
	// 入力画面を常に最前面に表示
	BOOL mIsTopMost = FALSE;
	// アクティブ状態でなくなったらウインドウを隠す
	BOOL mIsHideOnInactive = FALSE;
	// マウスカーソル位置に入力欄を表示する
	BOOL mIsShowMainWindowOnCursor = FALSE;


	
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



BasicSettingDialog::BasicSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("基本"), IDD_BASICSETTING, parentWnd),
	in(new PImpl)
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

	settingsPtr->Set(_T("HotKey:Modifiers"), (int)in->mHotKeyAttr.GetModifiers());
	settingsPtr->Set(_T("HotKey:VirtualKeyCode"), (int)in->mHotKeyAttr.GetVKCode());
	settingsPtr->Set(_T("HotKey:IsEnableHotKey"), in->mIsEnableHotKey);
	settingsPtr->Set(_T("HotKey:IsEnableModifierHotKey"), in->mIsEnableModifierHotKey);
	settingsPtr->Set(_T("HotKey:IsEnableModifierHotKeyOnRD"), in->mIsEnableModifierHotKeyOnRD);
	settingsPtr->Set(_T("HotKey:FirstModifierVirtualKeyCode"), (int)in->mModifierFirstVK);
	settingsPtr->Set(_T("HotKey:SecondModifierVirtualKeyCode"),(int) in->mModifierSecondVK);

	settingsPtr->Set(_T("Soyokaze:ShowToggle"), (bool)in->mIsShowToggle);
	settingsPtr->Set(_T("Soyokaze:IsIKeepTextWhenDlgHide"), (bool)in->mIsKeepTextWhenDlgHide);
	settingsPtr->Set(_T("Soyokaze:IsHideOnStartup"), (bool)in->mIsHideOnRun);
	settingsPtr->Set(_T("Soyokaze:TopMost"), (bool)in->mIsTopMost);
	settingsPtr->Set(_T("Soyokaze:IsHideOnInactive"), (bool)in->mIsHideOnInactive);

	bool isShowOnActWin = (in->mShowPositionType == POSTYPE_MOUSECURSOR);
	settingsPtr->Set(_T("Soyokaze:IsShowMainWindowOnCurorPos"), isShowOnActWin);
	bool isShowOnCursor = (in->mShowPositionType == POSTYPE_ACTIVEWINDOWCENTER);
	settingsPtr->Set(_T("Soyokaze:IsShowMainWindowOnActiveWindowCenter"), isShowOnCursor);

	__super::OnOK();
}

void BasicSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_HOTKEY, in->mHotKey);
	DDX_Check(pDX, IDC_CHECK_SHOWTOGGLE, in->mIsShowToggle);
	DDX_Check(pDX, IDC_CHECK_KEEPTEXTWHENDLGHIDE, in->mIsKeepTextWhenDlgHide);
	DDX_Check(pDX, IDC_CHECK_HIDEONRUN, in->mIsHideOnRun);
	DDX_Check(pDX, IDC_CHECK_TOPMOST, in->mIsTopMost);
	DDX_Check(pDX, IDC_CHECK_HIDEONINACTIVE, in->mIsHideOnInactive);
	DDX_CBIndex(pDX, IDC_COMBO_POSITION, in->mShowPositionType);
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
	if (in->mIsEnableHotKey) {
		text = in->mHotKeyAttr.ToString();
	}
	if (in->mIsEnableModifierHotKey) {
		if (text.IsEmpty() == FALSE) {
			text += _T(" / ");
		}
		text += AppHotKeyDialog::ToString(in->mModifierFirstVK, in->mModifierSecondVK);
	}
	in->mHotKey = text;

	return true;
}

void BasicSettingDialog::OnButtonHotKey()
{
	UpdateData();

	AppHotKeyDialog dlg(in->mHotKeyAttr, this);
	dlg.SetEnableHotKey(in->mIsEnableHotKey);
	dlg.SetEnableModifierHotKey(in->mIsEnableModifierHotKey);
	dlg.SetEnableModifierHotKeyOnRD(in->mIsEnableModifierHotKeyOnRD);
	dlg.SetModifierFirstVK(in->mModifierFirstVK);
	dlg.SetModifierSecondVK(in->mModifierSecondVK);

	if (dlg.DoModal() != IDOK) {
		return ;
	}
	dlg.GetAttribute(in->mHotKeyAttr);
	in->mIsEnableHotKey = dlg.IsEnableHotKey();
	in->mIsEnableModifierHotKey = dlg.IsEnableModifierHotKey();
	in->mIsEnableModifierHotKeyOnRD = dlg.IsEnableModifierHotKeyOnRD();
	in->mModifierFirstVK = dlg.GetModifierFirstVK();
	in->mModifierSecondVK = dlg.GetModifierSecondVK();

	UpdateStatus();
	UpdateData(FALSE);
}

void BasicSettingDialog::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	in->mHotKeyAttr = HOTKEY_ATTR(settingsPtr->Get(_T("HotKey:Modifiers"), MOD_ALT),
		                        settingsPtr->Get(_T("HotKey:VirtualKeyCode"), VK_SPACE));
	in->mHotKey = in->mHotKeyAttr.ToString();

	in->mIsEnableHotKey = settingsPtr->Get(_T("HotKey:IsEnableHotKey"), true);
	in->mIsEnableModifierHotKey = settingsPtr->Get(_T("HotKey:IsEnableModifierHotKey"), false);
	in->mIsEnableModifierHotKeyOnRD = settingsPtr->Get(_T("HotKey:IsEnableModifierHotKeyOnRD"), false);
	in->mModifierFirstVK = settingsPtr->Get(_T("HotKey:FirstModifierVirtualKeyCode"), VK_CONTROL);
	in->mModifierSecondVK = settingsPtr->Get(_T("HotKey:SecondModifierVirtualKeyCode"), VK_CONTROL);

	in->mIsShowToggle = settingsPtr->Get(_T("Soyokaze:ShowToggle"), true);
	in->mIsKeepTextWhenDlgHide = settingsPtr->Get(_T("Soyokaze:IsIKeepTextWhenDlgHide"), false);
	in->mIsHideOnRun = settingsPtr->Get(_T("Soyokaze:IsHideOnStartup"), false);
	in->mIsTopMost = settingsPtr->Get(_T("Soyokaze:TopMost"), false);
	in->mIsHideOnInactive = settingsPtr->Get(_T("Soyokaze:IsHideOnInactive"), false);

	bool isShowOnCursor = settingsPtr->Get(_T("Soyokaze:IsShowMainWindowOnCurorPos"), false);
	bool isShowOnActWin = settingsPtr->Get(_T("Soyokaze:IsShowMainWindowOnActiveWindowCenter"), false);
	if (isShowOnCursor) {
		in->mShowPositionType = POSTYPE_MOUSECURSOR;
	}
	else if (isShowOnActWin) {
		in->mShowPositionType = POSTYPE_ACTIVEWINDOWCENTER;
	}
	else {
		in->mShowPositionType = POSTYPE_KEEPLAST;
	}


}

bool BasicSettingDialog::GetHelpPageId(CString& id)
{
	id = _T("GeneralSetting");
	return true;
}

