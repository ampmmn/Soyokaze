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
	mHotKeyAttr(attr)
{
	SetHelpPageId(_T("HotKey"));
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
	DDX_Radio(pDX, IDC_RADIO_HOTKEY, mHotKeyType);
	DDX_CBIndex(pDX, IDC_COMBO_VKMODFIRST, mFirstVKIndex);
	DDX_CBIndex(pDX, IDC_COMBO_VKMODSECOND, mSecondVKIndex);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, mDescription);
}

BEGIN_MESSAGE_MAP(AppHotKeyDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_CHECK_SHIFT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_ALT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_CTRL, UpdateStatus)
	ON_COMMAND(IDC_CHECK_WIN, UpdateStatus)
	ON_COMMAND(IDC_RADIO_HOTKEY, UpdateStatus)
	ON_COMMAND(IDC_RADIO_HOTKEY2, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VK, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VKMODFIRST, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VKMODSECOND, UpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void AppHotKeyDialog::GetAttribute(HOTKEY_ATTR& attr)
{
	attr = mHotKeyAttr;
}

void AppHotKeyDialog::SetModifierHotKeyType(bool isModifierHotKey)
{
	mHotKeyType = isModifierHotKey ? 1 : 0;
}

bool AppHotKeyDialog::IsModifierHotKey()
{
	return mHotKeyType == 1;
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

void AppHotKeyDialog::UpdateStatus()
{
	UpdateData();
	// ToDo: 新しいコントロールに対する制御

	mDescription.Empty();

	bool isHotKey = (mHotKeyType == 0);
	GetDlgItem(IDC_CHECK_SHIFT)->EnableWindow(isHotKey);
	GetDlgItem(IDC_CHECK_CTRL)->EnableWindow(isHotKey);
	GetDlgItem(IDC_CHECK_ALT)->EnableWindow(isHotKey);
	GetDlgItem(IDC_CHECK_WIN)->EnableWindow(isHotKey);
	GetDlgItem(IDC_COMBO_VK)->EnableWindow(isHotKey);
	GetDlgItem(IDC_COMBO_VKMODFIRST)->EnableWindow(!isHotKey);
	GetDlgItem(IDC_COMBO_VKMODSECOND)->EnableWindow(!isHotKey);

	if (isHotKey) {
		// 予約済みのキーか?
		if (IsReservedKey(mHotKeyAttr)) {
			GetDlgItem(IDOK)->EnableWindow(false);
			mMessage.LoadString(IDS_ERR_HOTKEYRESERVED);
		}
		else {
			// ホットキーが既に使用されていないかチェック
			bool canRegister = mHotKeyAttr.TryRegister(GetSafeHwnd());
			GetDlgItem(IDOK)->EnableWindow(canRegister);

			mMessage.Empty();
			if (canRegister == false) {
				mMessage.LoadString(IDS_ERR_HOTKEYALREADYUSE);
			}
		}
	}
	else {
			mMessage.Empty();

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

	UpdateData(FALSE);
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
