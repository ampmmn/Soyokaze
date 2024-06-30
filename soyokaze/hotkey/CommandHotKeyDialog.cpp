#include "pch.h"
#include "framework.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "hotkey/CommandHotKeyManager.h"
#include "utility/Accessibility.h"
#include "resource.h"
#include <utility>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CommandHotKeyDialog::CommandHotKeyDialog(const CommandHotKeyAttribute& attr, CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_HOTKEY, parentWnd),
	mHotKeyAttr(attr)
{
	SetHelpPageId(_T("HotKey"));
}


CommandHotKeyDialog::~CommandHotKeyDialog()
{
}

void CommandHotKeyDialog::SetTargetName(const CString& name)
{
	mTargetName = name;
}

void CommandHotKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SHIFT, mHotKeyAttr.mUseShift);
	DDX_Check(pDX, IDC_CHECK_ALT, mHotKeyAttr.mUseAlt);
	DDX_Check(pDX, IDC_CHECK_CTRL, mHotKeyAttr.mUseCtrl);
	DDX_Check(pDX, IDC_CHECK_WIN, mHotKeyAttr.mUseWin);
	DDX_CBIndex(pDX, IDC_COMBO_VK, mHotKeyAttr.mVirtualKeyIdx);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_CBIndex(pDX, IDC_COMBO_TYPE, mHotKeyAttr.mIsGlobal);
}

BEGIN_MESSAGE_MAP(CommandHotKeyDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_CHECK_SHIFT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_ALT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_CTRL, UpdateStatus)
	ON_COMMAND(IDC_CHECK_WIN, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VK, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, UpdateStatus)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_CLEAR, OnButtonClear)
END_MESSAGE_MAP()

void CommandHotKeyDialog::GetAttribute(CommandHotKeyAttribute& attr)
{
	attr = mHotKeyAttr;
}

BOOL CommandHotKeyDialog::OnInitDialog()
{
	__super::OnInitDialog();

	if (mTargetName.IsEmpty() == FALSE) {
		CString caption;
		GetWindowText(caption);

		caption += _T(" - ");
		caption += mTargetName;
		caption += _T("コマンド");

		SetWindowText(caption);
	}

	// 初期値を覚えておく
	mHotKeyAttrInit = mHotKeyAttr;

	// 割り当て解除ボタンを有効にする
	GetDlgItem(IDC_BUTTON_CLEAR)->ShowWindow(SW_SHOW);

	UpdateStatus();

	return TRUE;
}

void CommandHotKeyDialog::UpdateStatus()
{
	UpdateData();

	GetDlgItem(IDC_CHECK_WIN)->EnableWindow(mHotKeyAttr.mIsGlobal);
	if (mHotKeyAttr.mIsGlobal == FALSE) {
		// ローカルホットキー(→キーアクセラレータ)の場合は、WINキーが使えないのでチェックを外す
		mHotKeyAttr.mUseWin = 0;
	}

	if (IsReservedKey(mHotKeyAttr)) {
		GetDlgItem(IDOK)->EnableWindow(false);
		mMessage.LoadString(IDS_ERR_HOTKEYRESERVED);
	}
	else {
		mMessage.Empty();
		if (mHotKeyAttr != mHotKeyAttrInit) {

			// 設定が初期値と異なる場合は、そのキーが使えるかどうかをチェックする

			if (mHotKeyAttr.IsUnmapped()) {
				// キー割り当てなし
				GetDlgItem(IDOK)->EnableWindow(TRUE);
				UpdateData(FALSE);
				return;
			}


			if (mHotKeyAttr.mIsGlobal) {
				bool canRegister = mHotKeyAttr.TryRegister(GetSafeHwnd());
				GetDlgItem(IDOK)->EnableWindow(canRegister);

				if (canRegister == false) {
					mMessage.LoadString(IDS_ERR_HOTKEYALREADYUSE);
				}
			}
			else {
				auto manager = launcherapp::core::CommandHotKeyManager::GetInstance();
				bool alreadUsed = manager->HasKeyBinding(mHotKeyAttr);
				if (alreadUsed) {
					mMessage.LoadString(IDS_ERR_HOTKEYALREADYUSE);
				}
				GetDlgItem(IDOK)->EnableWindow(alreadUsed == false);
			}
		}
	}

	UpdateData(FALSE);
}

/**
 *  エラーの時に一部コントロールの色を変える
 */
HBRUSH CommandHotKeyDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
bool CommandHotKeyDialog::IsReservedKey(const HOTKEY_ATTR& attr)
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

void CommandHotKeyDialog::OnButtonClear()
{
	UpdateData();
	mHotKeyAttr = CommandHotKeyAttribute();
	UpdateData(FALSE);

	UpdateStatus();
}

bool CommandHotKeyDialog::ShowDialog(const CString& name, CommandHotKeyAttribute& attr, CWnd* parentWnd)
{
	CommandHotKeyDialog dlg(attr, parentWnd);
	dlg.SetTargetName(name);
	if (dlg.DoModal() != IDOK) {
		return false;
	}
	dlg.GetAttribute(attr);
	return true;
}

