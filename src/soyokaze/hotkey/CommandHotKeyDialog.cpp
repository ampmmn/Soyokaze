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

enum {
	TYPE_LOCAL,       // アクティブなときだけ有効
	TYPE_LOCAL_SANDS, // アクティブなときだけ有効(SandS)
	TYPE_GLOBAL,      // 常に有効
};

CommandHotKeyDialog::CommandHotKeyDialog(const CommandHotKeyAttribute& attr, CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_COMMAND_HOTKEY, parentWnd),
	mHotKeyType(TYPE_LOCAL), mHotKeyAttr(attr)
{
	SetHelpPageId("HotKey");

	mIsUseHotKey = TRUE;
	if (attr.IsValid()) {
		if (attr.IsGlobal()) {
			mHotKeyType = TYPE_GLOBAL;
		}
		else {
			mHotKeyType = TYPE_LOCAL;
		}
		mVK =mHotKeyAttr.mHotKeyAttr.mVirtualKeyIdx;
	}
	else if (attr.IsValidSandS()) {
			mHotKeyType = TYPE_LOCAL_SANDS;
			mVK =mHotKeyAttr.mSandSKeyAttr.mVirtualKeyIdx;
	}
	else {
		mIsUseHotKey = FALSE;
		mVK =mHotKeyAttr.mHotKeyAttr.mVirtualKeyIdx;
	}
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
	DDX_Check(pDX, IDC_CHECK_HOTKEY, mIsUseHotKey);
	DDX_CBIndex(pDX, IDC_COMBO_TYPE, mHotKeyType);
	DDX_Check(pDX, IDC_CHECK_SHIFT, mHotKeyAttr.mHotKeyAttr.mUseShift);
	DDX_Check(pDX, IDC_CHECK_ALT, mHotKeyAttr.mHotKeyAttr.mUseAlt);
	DDX_Check(pDX, IDC_CHECK_CTRL, mHotKeyAttr.mHotKeyAttr.mUseCtrl);
	DDX_Check(pDX, IDC_CHECK_WIN, mHotKeyAttr.mHotKeyAttr.mUseWin);
	DDX_CBIndex(pDX, IDC_COMBO_SANDSMOD, mHotKeyAttr.mSandSKeyAttr.mModifier);
	DDX_CBIndex(pDX, IDC_COMBO_VK, mVK);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
}

BEGIN_MESSAGE_MAP(CommandHotKeyDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_CHECK_HOTKEY, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, UpdateStatus)
	ON_COMMAND(IDC_CHECK_SHIFT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_ALT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_CTRL, UpdateStatus)
	ON_COMMAND(IDC_CHECK_WIN, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_SANDSMOD, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VK, UpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void CommandHotKeyDialog::GetAttribute(CommandHotKeyAttribute& attr)
{
	// 使用しない設定をクリアする
	// (CommandHotKeyAttributeクラス自体は使う/使わないという形で設定値を保持しないため)
	attr = mHotKeyAttr;
	if (mIsUseHotKey == FALSE) {
		attr.Reset();
		attr.ResetSandS();
		return;
	}
	if (mHotKeyType == TYPE_LOCAL || mHotKeyType == TYPE_GLOBAL) {
		attr.ResetSandS();
	}
	else {
		attr.Reset();
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

void CommandHotKeyDialog::OnOK()
{
	if (UpdateData() == FALSE) {
		return;
	}

	if (mHotKeyType == TYPE_LOCAL || mHotKeyType == TYPE_GLOBAL) {
		mHotKeyAttr.mHotKeyAttr.mVirtualKeyIdx = mVK;
		mHotKeyAttr.mIsGlobal = mHotKeyType == TYPE_GLOBAL;
	}
	else {
		mHotKeyAttr.mSandSKeyAttr.mVirtualKeyIdx = mVK;
	}


	__super::OnOK();
}

void CommandHotKeyDialog::UpdateStatus()
{
	UpdateData();

	mMessage.Empty();

	// 表示状態の制御
	BOOL isHotKeySelected = (mHotKeyType == TYPE_LOCAL || mHotKeyType == TYPE_GLOBAL);
	BOOL isSandSSelected = !isHotKeySelected;
	BOOL isLocal = mHotKeyType == TYPE_LOCAL;

	UpdateCtrlState(IDC_STATIC_FRAME, isHotKeySelected, mIsUseHotKey != FALSE);
	UpdateCtrlState(IDC_CHECK_SHIFT, isHotKeySelected, mIsUseHotKey != FALSE);
	UpdateCtrlState(IDC_CHECK_ALT, isHotKeySelected, mIsUseHotKey != FALSE);
	UpdateCtrlState(IDC_CHECK_CTRL, isHotKeySelected, mIsUseHotKey != FALSE);
	UpdateCtrlState(IDC_CHECK_WIN, isHotKeySelected, mIsUseHotKey != FALSE && isLocal == FALSE);

	UpdateCtrlState(IDC_STATIC_MODSANDS, isSandSSelected, mIsUseHotKey != FALSE);
	UpdateCtrlState(IDC_COMBO_SANDSMOD, isSandSSelected, mIsUseHotKey != FALSE);

	UpdateCtrlState(IDC_COMBO_VK, true, mIsUseHotKey != FALSE);
	UpdateCtrlState(IDC_COMBO_TYPE, true, mIsUseHotKey != FALSE);

	bool isOK = true;
	if (mIsUseHotKey) {
		if (mHotKeyType == TYPE_LOCAL || mHotKeyType == TYPE_GLOBAL) {
			isOK = UpdateStatusForHotKey();
		}
		else if (mHotKeyType == TYPE_LOCAL_SANDS) {
			isOK = UpdateStatusForSandS();
		}
	}

	GetDlgItem(IDOK)->EnableWindow(isOK);
	UpdateData(FALSE);
}

void CommandHotKeyDialog::UpdateCtrlState(UINT ctrlID,bool isShow, bool isEnable)
{
	auto ctrl = GetDlgItem(ctrlID);
	if (ctrl == nullptr) {
		return;
	}
	ctrl->ShowWindow(isShow ? SW_SHOW : SW_HIDE);
	ctrl->EnableWindow(isEnable);
}

bool CommandHotKeyDialog::UpdateStatusForHotKey()
{
	mHotKeyAttr.mHotKeyAttr.mVirtualKeyIdx = mVK;

	if (mHotKeyAttr.mIsGlobal == FALSE) {
		// ローカルホットキー(→キーアクセラレータ)の場合は、WINキーが使えないのでチェックを外す
		mHotKeyAttr.mHotKeyAttr.mUseWin = 0;
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
		mMessage = _T("キーを設定してください");
		return false;
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
	mHotKeyAttr.mSandSKeyAttr.mVirtualKeyIdx = mVK;

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
	if (::utility::IsHighContrastMode()) {
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

