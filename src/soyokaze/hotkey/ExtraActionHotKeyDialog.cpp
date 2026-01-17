#include "pch.h"
#include "framework.h"
#include "hotkey/ExtraActionHotKeyDialog.h"
#include "hotkey/CommandHotKeyManager.h"
#include "utility/Accessibility.h"
#include "resource.h"
#include <utility>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ExtraActionHotKeyDialog::ExtraActionHotKeyDialog(const HOTKEY_ATTR& attr, CWnd* parentWnd) : 
	launcherapp::control::SinglePageDialog(IDD_HOTKEY_EXTRAACTION, parentWnd),
	mHotKeyAttr(attr)
{
	SetHelpPageId("ExtraActionHotKey");
	mVK =mHotKeyAttr.mVirtualKeyIdx;
}


ExtraActionHotKeyDialog::~ExtraActionHotKeyDialog()
{
}

void ExtraActionHotKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SHIFT, mHotKeyAttr.mUseShift);
	DDX_Check(pDX, IDC_CHECK_ALT, mHotKeyAttr.mUseAlt);
	DDX_Check(pDX, IDC_CHECK_CTRL, mHotKeyAttr.mUseCtrl);
	DDX_Check(pDX, IDC_CHECK_WIN, mHotKeyAttr.mUseWin);
	DDX_CBIndex(pDX, IDC_COMBO_VK, mVK);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
}

BEGIN_MESSAGE_MAP(ExtraActionHotKeyDialog, launcherapp::control::SinglePageDialog)
	ON_COMMAND(IDC_CHECK_SHIFT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_ALT, UpdateStatus)
	ON_COMMAND(IDC_CHECK_CTRL, UpdateStatus)
	ON_COMMAND(IDC_CHECK_WIN, UpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_VK, UpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void ExtraActionHotKeyDialog::GetAttribute(HOTKEY_ATTR& attr)
{
	attr = mHotKeyAttr;
}

BOOL ExtraActionHotKeyDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// 初期値を覚えておく
	mHotKeyAttrInit = mHotKeyAttr;

	UpdateStatus();

	return TRUE;
}

void ExtraActionHotKeyDialog::OnOK()
{
	if (UpdateData() == FALSE) {
		return;
	}

	mHotKeyAttr.mVirtualKeyIdx = mVK;

	__super::OnOK();
}

void ExtraActionHotKeyDialog::UpdateStatus()
{
	UpdateData();

	mMessage.Empty();

	bool isOK = UpdateStatusForHotKey();

	GetDlgItem(IDOK)->EnableWindow(isOK);
	UpdateData(FALSE);
}

bool ExtraActionHotKeyDialog::UpdateStatusForHotKey()
{
	mHotKeyAttr.mVirtualKeyIdx = mVK;
	// ローカルホットキー(→キーアクセラレータ)の場合は、WINキーが使えないのでチェックを強制的に外す
	mHotKeyAttr.mUseWin = 0;

	if (mHotKeyAttr.IsValid() == false) {
		mMessage = _T("キーを設定してください");
		return false;
	}

	if (mHotKeyAttr.IsReservedKey()) {
		// 予約済みキー
		mMessage.LoadString(IDS_ERR_HOTKEYRESERVED);
		return false;
	}

	if (mHotKeyAttr == mHotKeyAttrInit) {
		return true;
	}

	// 設定が初期値と異なる場合は、そのキーが使えるかどうかをチェックする
	if (mHotKeyAttr.IsUnmapped()) {
		// キー割り当てなし
		mMessage = _T("キーを設定してください");
		return false;
	}

	auto manager = launcherapp::core::CommandHotKeyManager::GetInstance();
	bool alreadyUsed = manager->HasKeyBinding(mHotKeyAttr);
	if (alreadyUsed) {
		mMessage.LoadString(IDS_ERR_HOTKEYALREADYUSE);
	}
	return alreadyUsed == false;
}

/**
 *  エラーの時に一部コントロールの色を変える
 */
HBRUSH ExtraActionHotKeyDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

bool ExtraActionHotKeyDialog::ShowDialog(HOTKEY_ATTR& attr, CWnd* parentWnd)
{
	ExtraActionHotKeyDialog dlg(attr, parentWnd);
	if (dlg.DoModal() != IDOK) {
		return false;
	}
	dlg.GetAttribute(attr);
	return true;
}

