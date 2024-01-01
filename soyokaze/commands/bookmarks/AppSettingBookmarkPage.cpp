#include "pch.h"
#include "framework.h"
#include "AppSettingBookmarkPage.h"
#include "Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
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

	__super::OnOK();
}

void AppSettingBookmarkPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_ENABLE_BOOKMARKS, mIsEnableBookmarks);
	DDX_Check(pDX, IDC_CHECK_USEURL, mIsUseURL);
	DDX_Check(pDX, IDC_CHECK_ENABLE_HISTORY_CHROME, mIsEnableHistoryChrome);
	DDX_Check(pDX, IDC_CHECK_ENABLE_HISTORY_EDGE, mIsEnableHistoryEdge);
}

BEGIN_MESSAGE_MAP(AppSettingBookmarkPage, SettingPage)
	ON_COMMAND(IDC_CHECK_ENABLE_BOOKMARKS, OnCheckEnable)
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
	return true;
}

void AppSettingBookmarkPage::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	mIsEnableBookmarks = settingsPtr->Get(_T("Bookmarks:EnableBookmarks"), true);
	mIsUseURL = settingsPtr->Get(_T("Bookmarks:UseURL"), true);
	mIsEnableHistoryChrome = settingsPtr->Get(_T("Browser::EnableHistoryChrome"), false);
	mIsEnableHistoryEdge = settingsPtr->Get(_T("Browser::EnableHistoryEdge"), false);
}

void AppSettingBookmarkPage::OnCheckEnable()
{
	UpdateData();
	UpdateStatus();
}

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace soyokaze
