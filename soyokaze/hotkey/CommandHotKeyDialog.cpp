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


CommandHotKeyDialog::CommandHotKeyDialog(const HOTKEY_ATTR& attr, bool isGlobal) : 
	CDialogEx(IDD_HOTKEY),
	mHotKeyAttr(attr),
	mIsGlobal(isGlobal ? TRUE : FALSE)
{
}


CommandHotKeyDialog::~CommandHotKeyDialog()
{
}

void CommandHotKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SHIFT, mHotKeyAttr.mUseShift);
	DDX_Check(pDX, IDC_CHECK_ALT, mHotKeyAttr.mUseAlt);
	DDX_Check(pDX, IDC_CHECK_CTRL, mHotKeyAttr.mUseCtrl);
	DDX_Check(pDX, IDC_CHECK_WIN, mHotKeyAttr.mUseWin);
	DDX_CBIndex(pDX, IDC_COMBO_VK, mHotKeyAttr.mVirtualKeyIdx);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_CBIndex(pDX, IDC_COMBO_TYPE, mIsGlobal);
}

BEGIN_MESSAGE_MAP(CommandHotKeyDialog, CDialogEx)
	ON_COMMAND(IDC_CHECK_SHIFT, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ALT, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_CTRL, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_WIN, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VK, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, OnUpdateStatus)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_CLEAR, OnButtonClear)
END_MESSAGE_MAP()

void CommandHotKeyDialog::GetAttribute(HOTKEY_ATTR& attr)
{
	attr = mHotKeyAttr;
}

bool CommandHotKeyDialog::IsGlobal()
{
	return mIsGlobal != FALSE;
}

BOOL CommandHotKeyDialog::OnInitDialog()
{
	__super::OnInitDialog();
	_LANG_WINDOW(GetSafeHwnd());

	// 初期値を覚えておく
	mHotKeyAttrInit = mHotKeyAttr;

	// 割り当て解除ボタンを有効にする
	GetDlgItem(IDC_BUTTON_CLEAR)->ShowWindow(SW_SHOW);

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void CommandHotKeyDialog::UpdateStatus()
{
	GetDlgItem(IDC_CHECK_WIN)->EnableWindow(mIsGlobal);
	if (mIsGlobal == FALSE) {
		// ローカルホットキー(→キーアクセラレータ)の場合は、WINキーが使えないのでチェックを外す
		mHotKeyAttr.mUseWin = 0;
	}

	if (IsReservedKey(mHotKeyAttr)) {
		GetDlgItem(IDOK)->EnableWindow(false);
		mMessage = _LANG_T("Keys used for text input are not available.");
	}
	else {
		mMessage.Empty();
		if (mHotKeyAttr != mHotKeyAttrInit) {

			// 設定が初期値と異なる場合は、そのキーが使えるかどうかをチェックする

			if (mHotKeyAttr.IsUnmapped()) {
				// キー割り当てなし
				GetDlgItem(IDOK)->EnableWindow(TRUE);
				return;
			}


			if (mIsGlobal) {
				bool canRegister = mHotKeyAttr.TryRegister(GetSafeHwnd());
				GetDlgItem(IDOK)->EnableWindow(canRegister);

				if (canRegister == false) {
					mMessage = _T("The key is already in use.");
				}
			}
			else {
				auto manager = soyokaze::core::CommandHotKeyManager::GetInstance();
				bool alreadUsed = manager->HasKeyBinding(mHotKeyAttr);
				if (alreadUsed) {
					mMessage = _T("The key is already in use.");
				}
				GetDlgItem(IDOK)->EnableWindow(alreadUsed == false);
			}
		}
	}
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

void CommandHotKeyDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandHotKeyDialog::OnButtonClear()
{
	UpdateData();
	mHotKeyAttr = HOTKEY_ATTR();
	UpdateStatus();
	UpdateData(FALSE);
}

