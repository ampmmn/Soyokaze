//あ
#include "pch.h"
#include "framework.h"
#include "AppSettingBookmarkPage.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace bookmarks {

AppSettingBookmarkPage::AppSettingBookmarkPage(CWnd* parentWnd) : 
	SettingPage(_T("ブラウザ関連"), IDD_APPSETTING_BOOKMARK, parentWnd)
{
}

AppSettingBookmarkPage::~AppSettingBookmarkPage()
{
}

BOOL AppSettingBookmarkPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL AppSettingBookmarkPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void AppSettingBookmarkPage::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("Bookmarks:EnableBookmarks"), (bool)mIsEnableBookmarks);
	settingsPtr->Set(_T("Bookmarks:UseURL"), (bool)mIsUseURL);
	settingsPtr->Set(_T("Browser::EnableHistoryChrome"), (bool)mIsEnableHistoryChrome);
	settingsPtr->Set(_T("Browser::EnableHistoryEdge"), (bool)mIsEnableHistoryEdge);
	settingsPtr->Set(_T("Browser::Timeout"), mTimeout);
	settingsPtr->Set(_T("Browser::Candidates"), mCandidates);
	settingsPtr->Set(_T("Browser::UseMigemo"), (bool)mIsUseMigemo);
	settingsPtr->Set(_T("Browser::UseURL"), (bool)mIsUseURLForHistory);

	__super::OnOK();
}

void AppSettingBookmarkPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_ENABLE_BOOKMARKS, mIsEnableBookmarks);
	DDX_Check(pDX, IDC_CHECK_USEURL, mIsUseURL);
	DDX_Check(pDX, IDC_CHECK_ENABLE_HISTORY_CHROME, mIsEnableHistoryChrome);
	DDX_Check(pDX, IDC_CHECK_ENABLE_HISTORY_EDGE, mIsEnableHistoryEdge);
	DDX_Text(pDX, IDC_EDIT_TIMEOUT, mTimeout);
	DDV_MinMaxInt(pDX, mTimeout, 0, 1000);
	DDX_Text(pDX, IDC_EDIT_CANDIDATES, mCandidates);
	DDV_MinMaxInt(pDX, mCandidates, 0, 32);
	DDX_Check(pDX, IDC_CHECK_USE_MIGEMO, mIsUseMigemo);
	DDX_Check(pDX, IDC_CHECK_USEURL2, mIsUseURLForHistory);
}

BEGIN_MESSAGE_MAP(AppSettingBookmarkPage, SettingPage)
	ON_COMMAND(IDC_CHECK_ENABLE_BOOKMARKS, OnCheckEnable)
	ON_COMMAND(IDC_CHECK_ENABLE_HISTORY_CHROME, OnCheckEnable)
	ON_COMMAND(IDC_CHECK_ENABLE_HISTORY_EDGE, OnCheckEnable)
END_MESSAGE_MAP()


BOOL AppSettingBookmarkPage::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool AppSettingBookmarkPage::UpdateStatus()
{
	GetDlgItem(IDC_CHECK_USEURL)->EnableWindow(mIsEnableBookmarks);

	BOOL isUseHistory = mIsEnableHistoryChrome || mIsEnableHistoryEdge;

	GetDlgItem(IDC_EDIT_TIMEOUT)->EnableWindow(isUseHistory);
	GetDlgItem(IDC_EDIT_CANDIDATES)->EnableWindow(isUseHistory);
	GetDlgItem(IDC_CHECK_USE_MIGEMO)->EnableWindow(isUseHistory);
	GetDlgItem(IDC_CHECK_USEURL2)->EnableWindow(isUseHistory);

	return true;
}

void AppSettingBookmarkPage::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	mIsEnableBookmarks = settingsPtr->Get(_T("Bookmarks:EnableBookmarks"), true);
	mIsUseURL = settingsPtr->Get(_T("Bookmarks:UseURL"), true);
	mIsEnableHistoryChrome = settingsPtr->Get(_T("Browser::EnableHistoryChrome"), false);
	mIsEnableHistoryEdge = settingsPtr->Get(_T("Browser::EnableHistoryEdge"), false);
	mTimeout = settingsPtr->Get(_T("Browser::Timeout"), 150);
	mCandidates = settingsPtr->Get(_T("Browser::Candidates"), 8);
	mIsUseMigemo = settingsPtr->Get(_T("Browser::UseMigemo"), false);
	mIsUseURLForHistory = settingsPtr->Get(_T("Browser::UseURL"), true);
}

void AppSettingBookmarkPage::OnCheckEnable()
{
	UpdateData();
	UpdateStatus();
}

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

