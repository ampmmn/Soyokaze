#include "pch.h"
#include "framework.h"
#include "WatchPathCommandEditDialog.h"
#include "commands/common/CommandEditValidation.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "gui/FolderDialog.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace watchpath {


CommandEditDialog::CommandEditDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_WATCHPATH, parentWnd), mIsDisabled(FALSE)
{
	SetHelpPageId(_T("WatchPathEdit"));
	mNotifyMessage = _T("更新を検知");
}

CommandEditDialog::~CommandEditDialog()
{
}

void CommandEditDialog::SetOrgName(const CString& name)
{
	mOrgName = name;
}

void CommandEditDialog::SetName(const CString& name)
{
	mName = name;
}

void CommandEditDialog::SetDescription(const CString& desc)
{
	mDescription = desc;
}

void CommandEditDialog::SetNotifyMessage(const CString& msg)
{
	mNotifyMessage = msg;
}

void CommandEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mDescription);
	DDX_Text(pDX, IDC_EDIT_PATH, mPath);
	DDX_Text(pDX, IDC_EDIT_MESSAGE, mNotifyMessage);
	DDX_Check(pDX, IDC_CHECK_DISABLE, mIsDisabled);
}

BEGIN_MESSAGE_MAP(CommandEditDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PATH, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE1, OnButtonFileBrowse)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR1, OnButtonDirBrowse)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


BOOL CommandEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	SetIcon(IconLoader::Get()->LoadUnknownIcon(), FALSE);

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)mOrgName);

	caption += suffix;
	SetWindowText(caption);

	UpdateStatus();
	UpdateData(FALSE);

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	return TRUE;
}

bool CommandEditDialog::UpdateStatus()
{
	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::common::IsValidCommandName(mName, mOrgName, mMessage);
	if (isNameValid == false) {
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	if (mPath.IsEmpty()) {
		mMessage = _T("フォルダのパスを指定してください。");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	if (PathIsDirectory(mPath) == FALSE) {
		mMessage = _T("指定されたフォルダは存在しません。");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	return true;
}

void CommandEditDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH CommandEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CommandEditDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	if (PathIsDirectory(mPath) == FALSE) {
		return;
	}

	__super::OnOK();
}

void CommandEditDialog::OnButtonFileBrowse()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, mPath, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandEditDialog::OnButtonDirBrowse()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mPath, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

}
}
}

