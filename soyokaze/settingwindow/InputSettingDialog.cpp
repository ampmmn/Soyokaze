#include "pch.h"
#include "framework.h"
#include "InputSettingDialog.h"
#include "commands/builtin/MainDirCommand.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using MainDirCommand = launcherapp::commands::builtin::MainDirCommand;

InputSettingDialog::InputSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("入力"), IDD_INPUTSETTING, parentWnd)
{
}

InputSettingDialog::~InputSettingDialog()
{
}

BOOL InputSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL InputSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void InputSettingDialog::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("Soyokaze:IsIMEOffOnActive"), (bool)mIsIMEOff);
	settingsPtr->Set(_T("Soyokaze:IsIgnoreUNC"), (bool)mIsIgnoreUNC);
	settingsPtr->Set(_T("Soyokaze:IsEnableMigemo"), (bool)mIsEnableMigemo);

	__super::OnOK();
}

void InputSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_IMEOFF, mIsIMEOff);
	DDX_Check(pDX, IDC_CHECK_IGNOREUNC, mIsIgnoreUNC);
	DDX_Check(pDX, IDC_CHECK_ENABLEMIGEMO, mIsEnableMigemo);
}

BEGIN_MESSAGE_MAP(InputSettingDialog, SettingPage)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, OnNotifyLinkOpen)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_APPDIR, OnNotifyLinkOpen)
END_MESSAGE_MAP()


BOOL InputSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	CWnd* link2 = GetDlgItem(IDC_SYSLINK_APPDIR);
	ASSERT(link2);
	CString str;
	 link2->GetWindowText(str);

	TCHAR appDir[MAX_PATH];
	GetModuleFileName(nullptr, appDir, MAX_PATH_NTFS);
	PathRemoveFileSpec(appDir);
	str.Replace(_T("$APPDIR"), appDir);
	link2->SetWindowText(str);

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool InputSettingDialog::UpdateStatus()
{
	return true;
}

void InputSettingDialog::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();
	mIsIMEOff = settingsPtr->Get(_T("Soyokaze:IsIMEOffOnActive"), false);
	mIsIgnoreUNC = settingsPtr->Get(_T("Soyokaze:IsIgnoreUNC"), false);
	mIsEnableMigemo = settingsPtr->Get(_T("Soyokaze:IsEnableMigemo"), true);
}

void InputSettingDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	NMLINK* linkPtr = (NMLINK*)pNMHDR;

	if (linkPtr->hdr.idFrom == IDC_SYSLINK_APPDIR) {
		launcherapp::core::CommandParameter param;
		MainDirCommand cmd(_T("tmp"));
		cmd.Execute(param);
	}
	else {
		ShellExecute(0, _T("open"), linkPtr->item.szUrl,  0, 0,SW_NORMAL);
	}
	*pResult = 0;
}

