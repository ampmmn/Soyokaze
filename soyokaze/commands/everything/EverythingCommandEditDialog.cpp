#include "pch.h"
#include "EverythingCOmmandEditDialog.h"
#include "commands/everything/EverythingCommandParam.h"
#include "gui/FolderDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/CommandEditValidation.h"
#include "utility/Accessibility.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using Command =  launcherapp::core::Command;

namespace launcherapp {
namespace commands {
namespace everything {

struct SettingDialog::PImpl
{
	// 編集開始時のコマンド名
	CString mOrgName;
	// メッセージ欄
	CString mMessage;
	// プレビュー
	CString mPreview;

	// 編集対象パラメータ
	CommandParam mParam;

	// ホットキー(表示用)
	CString mHotKey;
	CommandHotKeyAttribute mHotKeyAttr;

};


SettingDialog::SettingDialog() : 
	launcherapp::gui::SinglePageDialog(IDD_EVERYTHINGEDIT), in(new PImpl)
{
	SetHelpPageId(_T("EverythingEdit"));
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetParam(const CommandParam& param)
{
	in->mOrgName = param.mName;
	in->mParam = param;
}

const CommandParam& SettingDialog::GetParam() const
{
	return in->mParam;
}

void SettingDialog::SetHotKeyAttribute(const CommandHotKeyAttribute& attr)
{
	in->mHotKeyAttr = attr;
}

void SettingDialog::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mHotKeyAttr;
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_BASEDIR, in->mParam.mBaseDir);
	DDX_Check(pDX, IDC_CHECK_REGEX, in->mParam.mIsRegex);
	DDX_Check(pDX, IDC_CHECK_MATCHCASE, in->mParam.mIsMatchCase);
	DDX_Text(pDX, IDC_EDIT_OTHERPARAM, in->mParam.mOtherParam);
	DDX_CBIndex(pDX, IDC_COMBO_TARGET, in->mParam.mTargetType);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, in->mHotKey);
	DDX_Text(pDX, IDC_STATIC_PREVIEW, in->mPreview);
}

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_BASEDIR, OnUpdateStatus)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_BASEDIR, OnButtonBaseDir)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_COMMAND(IDC_CHECK_REGEX, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_MATCHCASE, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_TARGET, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_OTHERPARAM, OnUpdateStatus)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_OPTION, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_OPTION, OnNotifyLinkOpen)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	GetDlgItem(IDC_BUTTON_BASEDIR)->SetWindowTextW(L"\U0001F4C2");

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool SettingDialog::UpdateStatus()
{
	in->mMessage.Empty();

	in->mHotKey = in->mHotKeyAttr.ToString();
	if (in->mHotKey.IsEmpty()) {
		in->mHotKey.LoadString(IDS_NOHOTKEY);
	}

	in->mPreview = in->mParam.BuildQueryString(_T("(検索ワード)"));

	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::common::IsValidCommandName(in->mParam.mName, in->mOrgName, in->mMessage);
	GetDlgItem(IDOK)->EnableWindow(isNameValid ? TRUE : FALSE);

	return isNameValid;
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

void SettingDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}
	__super::OnOK();
}

void SettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonBaseDir()
{
	UpdateData();
	CFolderDialog dlg(_T(""), in->mParam.mBaseDir, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	in->mParam.mBaseDir = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonHotKey()	
{
	UpdateData();

	if (CommandHotKeyDialog::ShowDialog(in->mParam.mName, in->mHotKeyAttr, this) == false) {
		return ;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

// 検索仕様ページを開く
void SettingDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	ShellExecute(0, _T("open"), _T("https://www.voidtools.com/support/everything/searching/"),  0, 0, SW_NORMAL);
	*pResult = 0;
}




} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

