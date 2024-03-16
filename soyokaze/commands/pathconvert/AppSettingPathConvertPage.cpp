#include "pch.h"
#include "framework.h"
#include "AppSettingPathConvertPage.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace pathconvert {

AppSettingPathConvertPage::AppSettingPathConvertPage(CWnd* parentWnd) : 
	SettingPage(_T("パス変換"), IDD_APPSETTING_PATHCONVERT, parentWnd)
{
}

AppSettingPathConvertPage::~AppSettingPathConvertPage()
{
}

bool AppSettingPathConvertPage::UpdateStatus()
{
	return true;
}

BOOL AppSettingPathConvertPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL AppSettingPathConvertPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void AppSettingPathConvertPage::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("PathConvert:IsEnableGitBashPath"), (bool)mIsEnableGitBashPath);
	settingsPtr->Set(_T("PathConvert:IsEnableFileProtol"), (bool)mIsEnableFileProtolPath);

	__super::OnOK();
}

void AppSettingPathConvertPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_ENABLE_GITBASH, mIsEnableGitBashPath);
	DDX_Check(pDX, IDC_CHECK_ENABLE_FILEPROTOCOL, mIsEnableFileProtolPath);
}

BEGIN_MESSAGE_MAP(AppSettingPathConvertPage, SettingPage)
END_MESSAGE_MAP()


BOOL AppSettingPathConvertPage::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void AppSettingPathConvertPage::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	mIsEnableGitBashPath = settingsPtr->Get(_T("PathConvert:IsEnableGitBashPath"), true);
	mIsEnableFileProtolPath = settingsPtr->Get(_T("PathConvert:IsEnableFileProtol"), true);
}

} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace soyokaze

