#include "pch.h"
#include "WebHistorySettingDialog.h"
#include "commands/webhistory/WebHistoryCommandParam.h"
#include "commands/common/CommandEditValidation.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace webhistory {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct SettingDialog::PImpl
{
	// 設定情報
	CommandParam mParam;
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

	CString mHotKey;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_WEBHISTORYEDIT, parentWnd),
	in(std::make_unique<PImpl>())
{
	SetHelpPageId(_T("WebHistorySetting"));
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetName(const CString& name)
{
	in->mParam.mName = name;
}

void SettingDialog::SetOriginalName(const CString& name)
{
	in->mOrgName = name;
}

void SettingDialog::SetParam(const Param& param)
{
	in->mParam = param;
}

const SettingDialog::Param&
SettingDialog::GetParam()
{
	return in->mParam;
}

void SettingDialog::ResetHotKey()
{
	in->mParam.mHotKeyAttr.Reset();
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_KEYWORD, in->mParam.mKeyword);
	DDX_Check(pDX, IDC_CHECK_ENABLE_HISTORY_CHROME, in->mParam.mIsEnableHistoryChrome);
	DDX_Check(pDX, IDC_CHECK_ENABLE_HISTORY_EDGE, in->mParam.mIsEnableHistoryEdge);
	DDX_Text(pDX, IDC_EDIT_TIMEOUT, in->mParam.mTimeout);
	DDV_MinMaxInt(pDX, in->mParam.mTimeout, 0, 1000);
	DDX_Text(pDX, IDC_EDIT_CANDIDATES, in->mParam.mLimit);
	DDV_MinMaxInt(pDX, in->mParam.mLimit, 0, 32);
	DDX_Check(pDX, IDC_CHECK_USE_MIGEMO, in->mParam.mIsUseMigemo);
	DDX_Check(pDX, IDC_CHECK_USEURL2, in->mParam.mIsUseURL);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, in->mHotKey);
}

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLE_HISTORY_CHROME, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLE_HISTORY_EDGE, OnUpdateStatus)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	CString caption;
	GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	UpdateStatus();
	UpdateData(FALSE);

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	return TRUE;
}

void SettingDialog::UpdateStatus()
{
	bool canPressOK = true;

	bool isBothDisabled = in->mParam.mIsEnableHistoryChrome == false &&
	                      in->mParam.mIsEnableHistoryEdge == false;

	bool isNameValid =
	 	launcherapp::commands::common::IsValidCommandName(in->mParam.mName, in->mOrgName, in->mMessage);
	if (isNameValid == false) {
		canPressOK = false;
	}
	else if (isBothDisabled) {
		in->mMessage = _T("EdgeかChrome少なくともいずれかを選択してください");
		canPressOK = false;
	}
	else {
		in->mMessage.Empty();
	}

	GetDlgItem(IDOK)->EnableWindow(canPressOK ? TRUE : FALSE);
}

void SettingDialog::OnOK()
{
	UpdateData();
	__super::OnOK();
}

void SettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}


HBRUSH SettingDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = in->mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

void SettingDialog::OnButtonHotKey()
{
	UpdateData();

	if (CommandHotKeyDialog::ShowDialog(in->mParam.mName, in->mParam.mHotKeyAttr, this) == false) {
		return ;
	}
	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	UpdateData(FALSE);
}

}
}
}
