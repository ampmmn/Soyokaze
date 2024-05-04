#include "pch.h"
#include "framework.h"
#include "HotKeyDialog.h"
#include "utility/Accessibility.h"
#include "resource.h"
#include <utility>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


HotKeyDialog::HotKeyDialog(const HOTKEY_ATTR& attr, CWnd* parent) : 
	launcherapp::gui::SinglePageDialog(IDD_HOTKEY, parent),
	mHotKeyAttr(attr)
{
	SetHelpPageId(_T("HotKey"));
}

HotKeyDialog::~HotKeyDialog()
{
}

void HotKeyDialog::SetTargetName(const CString& name)
{
	mTargetName = name;
}

void HotKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SHIFT, mHotKeyAttr.mUseShift);
	DDX_Check(pDX, IDC_CHECK_ALT, mHotKeyAttr.mUseAlt);
	DDX_Check(pDX, IDC_CHECK_CTRL, mHotKeyAttr.mUseCtrl);
	DDX_Check(pDX, IDC_CHECK_WIN, mHotKeyAttr.mUseWin);
	DDX_CBIndex(pDX, IDC_COMBO_VK, mHotKeyAttr.mVirtualKeyIdx);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
}

BEGIN_MESSAGE_MAP(HotKeyDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_CHECK_SHIFT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_ALT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_CTRL, UpdateStatus)
	ON_COMMAND(IDC_CHECK_WIN, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VK, UpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void HotKeyDialog::GetAttribute(HOTKEY_ATTR& attr)
{
	attr = mHotKeyAttr;
}

BOOL HotKeyDialog::OnInitDialog()
{
	__super::OnInitDialog();

	if (mTargetName.IsEmpty() == FALSE) {
		CString caption;
		GetWindowText(caption);

		caption += _T(" - ");
		caption += mTargetName;

		SetWindowText(caption);
	}

	// ダイアログリソース(IDD_HOTKEY)をコマンド用ホットキーと共有するが、
	// アプリのホットキー設定では使わない項目を非表示にする
	GetDlgItem(IDC_COMBO_TYPE)->ShowWindow(SW_HIDE);

	UpdateStatus();

	return TRUE;
}

void HotKeyDialog::UpdateStatus()
{
	UpdateData();

	if (IsReservedKey(mHotKeyAttr)) {
		GetDlgItem(IDOK)->EnableWindow(false);
		mMessage.LoadString(IDS_ERR_HOTKEYRESERVED);
	}
	else {
		bool canRegister = mHotKeyAttr.TryRegister(GetSafeHwnd());
		GetDlgItem(IDOK)->EnableWindow(canRegister);

		mMessage.Empty();
		if (canRegister == false) {
			mMessage.LoadString(IDS_ERR_HOTKEYALREADYUSE);
		}
	}

	UpdateData(FALSE);
}

/**
 *  エラーの時に一部コントロールの色を変える
 */
HBRUSH HotKeyDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
bool HotKeyDialog::IsReservedKey(const HOTKEY_ATTR& attr)
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

