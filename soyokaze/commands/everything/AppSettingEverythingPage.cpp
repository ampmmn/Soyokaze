//あ
#include "pch.h"
#include "framework.h"
#include "AppSettingEverythingPage.h"
#include "setting/Settings.h"
#include "app/Manual.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace everything {

AppSettingEverythingPage::AppSettingEverythingPage(CWnd* parentWnd) : 
	SettingPage(_T("Everything"), IDD_APPSETTING_EVERYTHING, parentWnd)
{
}

AppSettingEverythingPage::~AppSettingEverythingPage()
{
}

BOOL AppSettingEverythingPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}

	if (mIsRunApp == 1 && PathFileExists(mEverythingExePath) == FALSE) {
		CString msg;
		msg.Format(_T("%sは有効なパスではありません"), (LPCTSTR)mEverythingExePath);
		AfxMessageBox(msg);
		return FALSE;
	}

	return TRUE;
}

BOOL AppSettingEverythingPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void AppSettingEverythingPage::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("Everything:IsUseAPI"), mIsUseAPI != FALSE);
	settingsPtr->Set(_T("Everything:IsUseWM"), mIsUseWM != FALSE);
	settingsPtr->Set(_T("Everything:IsRunApp"), mIsRunApp != FALSE);
	settingsPtr->Set(_T("Everything:EverythingExePath"), mEverythingExePath);

	__super::OnOK();
}

void AppSettingEverythingPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_USEAPI, mIsUseAPI);
	DDX_Check(pDX, IDC_CHECK_USEWM, mIsUseWM);
	DDX_Check(pDX, IDC_CHECK_RUNEVERYTHING, mIsRunApp);
	DDX_Text(pDX, IDC_EDIT_EVERYTHINGEXEPATH, mEverythingExePath);
}

BEGIN_MESSAGE_MAP(AppSettingEverythingPage, SettingPage)
	ON_COMMAND(IDC_CHECK_RUNEVERYTHING, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE1, OnButtonBrowse)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_COMMAND, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_COMMAND, OnNotifyLinkOpen)
END_MESSAGE_MAP()


BOOL AppSettingEverythingPage::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool AppSettingEverythingPage::UpdateStatus()
{
	BOOL isRunApp = (mIsRunApp == 1) ? TRUE : FALSE;
	GetDlgItem(IDC_BUTTON_BROWSEFILE1)->EnableWindow(isRunApp);
	GetDlgItem(IDC_EDIT_EVERYTHINGEXEPATH)->EnableWindow(isRunApp);

	return true;
}

void AppSettingEverythingPage::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	mIsUseAPI = settingsPtr->Get(_T("Everything:IsUseAPI"), true);
	mIsUseWM = settingsPtr->Get(_T("Everything:IsUseWM"), true);
	mIsRunApp = settingsPtr->Get(_T("Everything:IsRunApp"), false);
	mEverythingExePath = settingsPtr->Get(_T("Everything:EverythingExePath"), _T(""));
}

bool AppSettingEverythingPage::GetHelpPageId(CString& id)
{
	id = _T("EverythingSetting");
	return true;
}


void AppSettingEverythingPage::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
}

void AppSettingEverythingPage::OnButtonBrowse()
{
	UpdateData();

	CString filterStr(_T("Everything.exe|Everything.exe|実行ファイル(*.exe)|*.exe||"));
	CFileDialog dlg(TRUE, NULL, mEverythingExePath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mEverythingExePath = dlg.GetPathName();
	UpdateData(FALSE);
}

// マニュアル表示
void AppSettingEverythingPage::OnNotifyLinkOpen(
	NMHDR *,
 	LRESULT *pResult
)
{
	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("EverythingEdit"));
	*pResult = 0;
}



} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

