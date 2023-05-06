#include "pch.h"
#include "framework.h"
#include "HotKeyDialog.h"
#include "resource.h"
#include <utility>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


HotKeyDialog::HotKeyDialog(const HOTKEY_ATTR& attr) : 
	CDialogEx(IDD_HOTKEY),
	mHotKeyAttr(attr)
{
}

HotKeyDialog::~HotKeyDialog()
{
}

void HotKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SHIFT, mHotKeyAttr.mUseShift);
	DDX_Check(pDX, IDC_CHECK_ALT, mHotKeyAttr.mUseAlt);
	DDX_Check(pDX, IDC_CHECK_CTRL, mHotKeyAttr.mUseCtrl);
	DDX_Check(pDX, IDC_CHECK_WIN, mHotKeyAttr.mUseWin);
	DDX_CBIndex(pDX, IDC_COMBO_VK, mHotKeyAttr.mVirtualKeyIdx);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
}

BEGIN_MESSAGE_MAP(HotKeyDialog, CDialogEx)
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

	UpdateStatus();

	return TRUE;
}

void HotKeyDialog::UpdateStatus()
{
	UpdateData();

	bool canRegister = mHotKeyAttr.TryRegister(GetSafeHwnd());
	GetDlgItem(IDOK)->EnableWindow(canRegister);

	mMessage.Empty();
	if (canRegister == false) {
		mMessage.LoadString(IDS_ERR_HOTKEYALREADYUSE);
	}

	UpdateData(FALSE);
}

/**
 *  エラーの時に一部コントロールの色を変える
 */
HBRUSH HotKeyDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}

	return br;
}
