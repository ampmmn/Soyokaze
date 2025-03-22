//あ
#include "pch.h"
#include "framework.h"
#include "AppSettingClipboardHistoryPage.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "app/Manual.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

AppSettingClipboardHistoryPage::AppSettingClipboardHistoryPage(CWnd* parentWnd) : 
	SettingPage(_T("クリップボード履歴"), IDD_APPSETTING_CLIPBOARD, parentWnd),
	mIsEnable(FALSE),
	mPrefix(_T("cb")),
	mNumOfResults(16),
	mSizeLimit(64),
	mCountLimit(1024),
	mInterval(500)
{
}

AppSettingClipboardHistoryPage::~AppSettingClipboardHistoryPage()
{
}

BOOL AppSettingClipboardHistoryPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}

	if (mIsEnable == FALSE) {
		return TRUE;
	}

	if (mPrefix.IsEmpty()) {
		AfxMessageBox(_T("プレフィックスを入力してください"));
		return FALSE;
	}

	// 正規表現として有効化をチェックする
	if (mExcludePattern.IsEmpty() == FALSE) {
		try {
			tregex regTmp((LPCTSTR)mExcludePattern);
		}
		catch(std::regex_error& e) {
			CString msg((LPCTSTR)IDS_ERR_INVALIDREGEXP);
			msg += _T("\n");

			CStringA what(e.what());
			msg += _T("\n");
			msg += (CString)what;
			msg += _T("\n");
			msg += mExcludePattern;
			AfxMessageBox(msg);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL AppSettingClipboardHistoryPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void AppSettingClipboardHistoryPage::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("ClipboardHistory:IsEnable"), (bool)mIsEnable);
	settingsPtr->Set(_T("ClipboardHistory:Prefix"), mPrefix);
	settingsPtr->Set(_T("ClipboardHistory:NumOfResults"), mNumOfResults);
	settingsPtr->Set(_T("ClipboardHistory:SizeLimit"), mSizeLimit);
	settingsPtr->Set(_T("ClipboardHistory:CountLimit"), mCountLimit);
	settingsPtr->Set(_T("ClipboardHistory:Interval"), mInterval);
	settingsPtr->Set(_T("ClipboardHistory:ExcludePattern"), mExcludePattern);

	__super::OnOK();
}

void AppSettingClipboardHistoryPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_ENABLE, mIsEnable);
	DDX_Text(pDX, IDC_EDIT_PREFIX, mPrefix);
	DDX_Text(pDX, IDC_EDIT_NUMOFRESULTS, mNumOfResults);
	DDV_MinMaxInt(pDX, mNumOfResults, 1, 128);
	DDX_Text(pDX, IDC_EDIT_SIZELIMIT, mSizeLimit);
	DDV_MinMaxInt(pDX, mSizeLimit, 1, 128);
	DDX_Text(pDX, IDC_EDIT_COUNTLIMIT, mCountLimit);
	DDV_MinMaxInt(pDX, mCountLimit, 1, 131072);
	DDX_Text(pDX, IDC_EDIT_INTERVAL, mInterval);
	DDV_MinMaxInt(pDX, mInterval, 0, 5000);
	DDX_Text(pDX, IDC_EDIT_EXCLUDE, mExcludePattern);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AppSettingClipboardHistoryPage, SettingPage)
	ON_COMMAND(IDC_CHECK_ENABLE, OnUpdateStatus)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL AppSettingClipboardHistoryPage::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool AppSettingClipboardHistoryPage::UpdateStatus()
{
	BOOL isEnable = (mIsEnable == 1) ? TRUE : FALSE;
	GetDlgItem(IDC_EDIT_PREFIX)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_NUMOFRESULTS)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_SIZELIMIT)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_COUNTLIMIT)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_INTERVAL)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_EXCLUDE)->EnableWindow(isEnable);

	return true;
}

void AppSettingClipboardHistoryPage::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	mIsEnable = settingsPtr->Get(_T("ClipboardHistory:IsEnable"), false);
	mPrefix = settingsPtr->Get(_T("ClipboardHistory:Prefix"), _T("cb"));
	mNumOfResults = settingsPtr->Get(_T("ClipboardHistory:NumOfResults"), 16);
	mSizeLimit = settingsPtr->Get(_T("ClipboardHistory:SizeLimit"), 64);
	mCountLimit = settingsPtr->Get(_T("ClipboardHistory:CountLimit"), 1024);
	mInterval = settingsPtr->Get(_T("ClipboardHistory:Interval"), 500);
	mExcludePattern = settingsPtr->Get(_T("ClipboardHistory:ExcludePattern"), _T(""));
}

bool AppSettingClipboardHistoryPage::GetHelpPageId(CString& id)
{
	id = _T("ClipboardHistorySetting");
	return true;
}


void AppSettingClipboardHistoryPage::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
}

} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

