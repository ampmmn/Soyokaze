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
	launcherapp::gui::SinglePageDialog(IDD_COMMAND_HOTKEY, parentWnd),
	mHotKeyAttr(attr)
{
	SetHelpPageId(_T("HotKey"));

	mIsUseHotKey = attr.IsValid();
	mIsUseSandS = attr.IsValidSandS();
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
	DDX_Check(pDX, IDC_CHECK_SHIFT, mHotKeyAttr.mHotKeyAttr.mUseShift);
	DDX_Check(pDX, IDC_CHECK_ALT, mHotKeyAttr.mHotKeyAttr.mUseAlt);
	DDX_Check(pDX, IDC_CHECK_CTRL, mHotKeyAttr.mHotKeyAttr.mUseCtrl);
	DDX_Check(pDX, IDC_CHECK_WIN, mHotKeyAttr.mHotKeyAttr.mUseWin);
	DDX_CBIndex(pDX, IDC_COMBO_VK, mHotKeyAttr.mHotKeyAttr.mVirtualKeyIdx);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_CBIndex(pDX, IDC_COMBO_TYPE, mHotKeyAttr.mIsGlobal);
	DDX_Check(pDX, IDC_CHECK_HOTKEY, mIsUseHotKey);
	DDX_Check(pDX, IDC_CHECK_SANDSHOTKEY, mIsUseSandS);
	DDX_CBIndex(pDX, IDC_COMBO_SANDSMOD, mHotKeyAttr.mSandSKeyAttr.mModifier);
	DDX_CBIndex(pDX, IDC_COMBO_SANDSVK, mHotKeyAttr.mSandSKeyAttr.mVirtualKeyIdx);
}

BEGIN_MESSAGE_MAP(CommandHotKeyDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_CHECK_SHIFT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_ALT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_CTRL, UpdateStatus)
	ON_COMMAND(IDC_CHECK_WIN, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VK, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, UpdateStatus)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_CHECK_HOTKEY, UpdateStatus)
	ON_COMMAND(IDC_CHECK_SANDSHOTKEY, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_SANDSMOD, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_SANDSVK, UpdateStatus)
END_MESSAGE_MAP()

void CommandHotKeyDialog::GetAttribute(CommandHotKeyAttribute& attr)
{
	attr = mHotKeyAttr;
	if (mIsUseHotKey == FALSE) {
		attr.Reset();
	}
	if (mIsUseSandS == FALSE) {
		attr.ResetSandS();
	}
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

	UpdateStatus();

	return TRUE;
}

void CommandHotKeyDialog::UpdateStatus()
{
	UpdateData();

	GetDlgItem(IDC_CHECK_WIN)->EnableWindow(mIsUseHotKey && mHotKeyAttr.mIsGlobal);
	GetDlgItem(IDC_CHECK_SHIFT)->EnableWindow(mIsUseHotKey);
	GetDlgItem(IDC_CHECK_ALT)->EnableWindow(mIsUseHotKey);
	GetDlgItem(IDC_CHECK_CTRL)->EnableWindow(mIsUseHotKey);
	GetDlgItem(IDC_COMBO_VK)->EnableWindow(mIsUseHotKey);
	GetDlgItem(IDC_COMBO_TYPE)->EnableWindow(mIsUseHotKey);
	GetDlgItem(IDC_COMBO_SANDSMOD)->EnableWindow(mIsUseSandS);
	GetDlgItem(IDC_COMBO_SANDSVK)->EnableWindow(mIsUseSandS);

	mMessage.Empty();

	bool isOK = UpdateStatusForHotKey();
	if (isOK) {
		isOK = UpdateStatusForSandS();
	}

	GetDlgItem(IDOK)->EnableWindow(isOK);
	UpdateData(FALSE);
}

bool CommandHotKeyDialog::UpdateStatusForHotKey()
{
	if (mHotKeyAttr.mIsGlobal == FALSE) {
		// ローカルホットキー(→キーアクセラレータ)の場合は、WINキーが使えないのでチェックを外す
		mHotKeyAttr.mHotKeyAttr.mUseWin = 0;
	}

	if (mIsUseHotKey == FALSE) {
		return true;
	}

	if (mHotKeyAttr.mHotKeyAttr.IsValid() == false) {
		mMessage = _T("キーを設定してください");
		return false;
	}

	if (mHotKeyAttr.mHotKeyAttr.IsReservedKey()) {
		// 予約済みキー
		mMessage.LoadString(IDS_ERR_HOTKEYRESERVED);
		return false;
	}

	if (mHotKeyAttr.mHotKeyAttr == mHotKeyAttrInit.mHotKeyAttr) {
		return true;
	}

	// 設定が初期値と異なる場合は、そのキーが使えるかどうかをチェックする
	if (mHotKeyAttr.mHotKeyAttr.IsUnmapped()) {
		// キー割り当てなし
		return true;
	}

	if (mHotKeyAttr.mIsGlobal) {
		// グローバルホットキーの場合
		bool canRegister = mHotKeyAttr.mHotKeyAttr.TryRegister(GetSafeHwnd());
		if (canRegister == false) {
			mMessage.LoadString(IDS_ERR_HOTKEYALREADYUSE);
		}
		return canRegister;
	}
	else {
		// ローカルホットキーの場合
		auto manager = launcherapp::core::CommandHotKeyManager::GetInstance();
		bool alreadyUsed = manager->HasKeyBinding(mHotKeyAttr.mHotKeyAttr);
		if (alreadyUsed) {
			mMessage.LoadString(IDS_ERR_HOTKEYALREADYUSE);
		}
		return alreadyUsed == false;
	}
}

bool CommandHotKeyDialog::UpdateStatusForSandS()
{
	if (mIsUseSandS == FALSE) {
		return true;
	}

	const auto& sandsAttr = mHotKeyAttr.mSandSKeyAttr;
	if (sandsAttr.IsValid() == false) {
		mMessage = _T("キーを設定してください");
		return false;
	}

	if (mHotKeyAttr.mSandSKeyAttr == mHotKeyAttrInit.mSandSKeyAttr) {
		return true;
	}

	auto manager = launcherapp::core::CommandHotKeyManager::GetInstance();
	bool alreadyUsed = manager->HasKeyBinding(sandsAttr);
	if (alreadyUsed) {
		mMessage = _T("指定されたSandS設定は他のコマンドで既に使用されています");
		return false;
	}

	return true;
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

