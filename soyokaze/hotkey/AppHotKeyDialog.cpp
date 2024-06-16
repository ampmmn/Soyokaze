#include "pch.h"
#include "framework.h"
#include "AppHotKeyDialog.h"
#include "utility/Accessibility.h"
#include "resource.h"
#include <utility>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


AppHotKeyDialog::AppHotKeyDialog(const HOTKEY_ATTR& attr, CWnd* parent) : 
	launcherapp::gui::SinglePageDialog(IDD_LAUNCHER_HOTKEY, parent),
	mHotKeyAttr(attr),
	mIsEnableHotKey(TRUE),
	mIsEnableModifieHotKey(FALSE)
{
	SetHelpPageId(_T("AppHotKey"));
}

AppHotKeyDialog::~AppHotKeyDialog()
{
}

void AppHotKeyDialog::SetTargetName(const CString& name)
{
	mTargetName = name;
}

void AppHotKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SHIFT, mHotKeyAttr.mUseShift);
	DDX_Check(pDX, IDC_CHECK_ALT, mHotKeyAttr.mUseAlt);
	DDX_Check(pDX, IDC_CHECK_CTRL, mHotKeyAttr.mUseCtrl);
	DDX_Check(pDX, IDC_CHECK_WIN, mHotKeyAttr.mUseWin);
	DDX_CBIndex(pDX, IDC_COMBO_VK, mHotKeyAttr.mVirtualKeyIdx);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Check(pDX, IDC_CHECK_HOTKEY, mIsEnableHotKey);
	DDX_Check(pDX, IDC_CHECK_HOTKEY2, mIsEnableModifieHotKey);
	DDX_CBIndex(pDX, IDC_COMBO_VKMODFIRST, mFirstVKIndex);
	DDX_CBIndex(pDX, IDC_COMBO_VKMODSECOND, mSecondVKIndex);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, mDescription);
}

BEGIN_MESSAGE_MAP(AppHotKeyDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_CHECK_SHIFT, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ALT, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_CTRL, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_WIN, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_HOTKEY, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_HOTKEY2, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VK, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VKMODFIRST, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VKMODSECOND, OnUpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void AppHotKeyDialog::GetAttribute(HOTKEY_ATTR& attr)
{
	attr = mHotKeyAttr;
}

bool AppHotKeyDialog::IsEnableHotKey()
{
	return mIsEnableHotKey != FALSE;
}

bool AppHotKeyDialog::IsEnableModifierHotKey()
{
	return mIsEnableModifieHotKey != FALSE;
}

void AppHotKeyDialog::SetEnableHotKey(bool isEnable)
{
	mIsEnableHotKey = isEnable ? TRUE : FALSE;
}

void AppHotKeyDialog::SetEnableModifierHotKey(bool isEnable)
{
	mIsEnableModifieHotKey = isEnable ? TRUE : FALSE;
}

static int VKToCBIndex(UINT vk)
{
	switch(vk) {
	case VK_MENU:     return 0;
	case VK_CONTROL: return 1;
	case VK_SHIFT:   return 2;
	case VK_LWIN:    return 3;
	default:         return 1;
	}
}

static UINT CBIndexToVK(int idx)
{
	switch(idx) {
	case 0:    return VK_MENU;
	case 1:    return VK_CONTROL;
	case 2:    return VK_SHIFT;
	case 3:    return VK_LWIN;
	default:   return VK_CONTROL;
	}
}

void AppHotKeyDialog::SetModifierFirstVK(UINT vk)
{
	mFirstVKIndex = VKToCBIndex(vk);
}

void AppHotKeyDialog::SetModifierSecondVK(UINT vk)
{
	mSecondVKIndex = VKToCBIndex(vk);
}

UINT AppHotKeyDialog::GetModifierFirstVK()
{
	return CBIndexToVK(mFirstVKIndex);
}

UINT AppHotKeyDialog::GetModifierSecondVK()
{
	return CBIndexToVK(mSecondVKIndex);
}


BOOL AppHotKeyDialog::OnInitDialog()
{
	__super::OnInitDialog();

	if (mTargetName.IsEmpty() == FALSE) {
		CString caption;
		GetWindowText(caption);

		caption += _T(" - ");
		caption += mTargetName;

		SetWindowText(caption);
	}

	UpdateStatus();

	return TRUE;
}

static CString GetVKComboText(int n)
{
	switch(n) {
		case 0: return _T("Alt");
		case 1: return _T("Control");
		case 2: return _T("Shift");
		case 3: return _T("Win");
		default: return _T("?");
	}
}

void AppHotKeyDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

bool AppHotKeyDialog::UpdateStatus()
{
	mDescription.Empty();

	GetDlgItem(IDC_CHECK_SHIFT)->EnableWindow(mIsEnableHotKey);
	GetDlgItem(IDC_CHECK_CTRL)->EnableWindow(mIsEnableHotKey);
	GetDlgItem(IDC_CHECK_ALT)->EnableWindow(mIsEnableHotKey);
	GetDlgItem(IDC_CHECK_WIN)->EnableWindow(mIsEnableHotKey);
	GetDlgItem(IDC_COMBO_VK)->EnableWindow(mIsEnableHotKey);

	GetDlgItem(IDC_COMBO_VKMODFIRST)->EnableWindow(mIsEnableModifieHotKey);
	GetDlgItem(IDC_COMBO_VKMODSECOND)->EnableWindow(mIsEnableModifieHotKey);

	if (mIsEnableHotKey == FALSE && mIsEnableModifieHotKey == FALSE) {
		mMessage = _T("いずれかのホットキーを有効にする必要があります");
		return false;
	}

	bool isOK = false;
	if (mIsEnableHotKey) {
		// 予約済みのキーか?
		if (IsReservedKey(mHotKeyAttr)) {
			GetDlgItem(IDOK)->EnableWindow(false);
			mMessage.LoadString(IDS_ERR_HOTKEYRESERVED);
		}
		else {
			// ホットキーが既に使用されていないかチェック
			bool canRegister = mHotKeyAttr.TryRegister(GetSafeHwnd());
			GetDlgItem(IDOK)->EnableWindow(canRegister);

			if (canRegister == false) {
				mMessage.LoadString(IDS_ERR_HOTKEYALREADYUSE);
			}
			else {
				mMessage.Empty();
				isOK = true;
			}
		}
	}

	if (mIsEnableModifieHotKey) {

			mMessage.Empty();
			isOK = true;

			bool isSame = (mFirstVKIndex == mSecondVKIndex);

			if (isSame) {
				mDescription.Format(_T("%sキーを2回押す"), (LPCTSTR)GetVKComboText(mFirstVKIndex));
			}
			else {
				mDescription.Format(_T("%sキーと%sキーを同時押し"),
					 	(LPCTSTR)GetVKComboText(mFirstVKIndex),
					 	(LPCTSTR)GetVKComboText(mSecondVKIndex));
			}
	}

	return isOK;
}

/**
 *  エラーの時に一部コントロールの色を変える
 */
HBRUSH AppHotKeyDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}

	return br;
}

// 利用できないキーか?
bool AppHotKeyDialog::IsReservedKey(const HOTKEY_ATTR& attr)
{
	if (attr.IsValid() == false) {
		return false;
	}

	if (attr.GetModifiers() == 0) {

		// 無修飾、かつ、Num0-9キーとFunctionキー以外のキーは割り当てを許可しない
		// (横取りすると通常の入力に差し支えあるので)
		if (attr.IsNumKey() == false && attr.IsFunctionKey() == false) {
			return true;
		}
	}
	if (attr.GetModifiers() == MOD_SHIFT) {
		// Shift+英字キーも許可しない
		// (横取りすると通常の入力に差し支えあるので)
		if (attr.IsAlphabetKey()) {
			return true;
		}
	}

	return false;
}

CString AppHotKeyDialog::ToString(UINT firstVK, UINT secondVK)
{
	bool isSame = firstVK == secondVK;

	CString str;
	if (isSame) {
		str.Format(_T("%sキーを2回押す"), (LPCTSTR)GetVKComboText(VKToCBIndex(firstVK)));
	}
	else {
		str.Format(_T("%sキーと%sキー同時押し"),
				(LPCTSTR)GetVKComboText(VKToCBIndex(firstVK)),
				(LPCTSTR)GetVKComboText(VKToCBIndex(secondVK)));
	}
	return str;
}
