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

class AppSettingPage : public CDialog
{
public:
	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	bool UpdateStatus();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();

public:
	BOOL mIsEnable{FALSE};
	CString mPrefix{_T("cb")};
	int mNumOfResults{16};
	int mSizeLimit{4};
	int mCountLimit{1024};
	int mInterval{500};
	CString mExcludePattern;
	
	Settings* mSettingsPtr{nullptr};
};

bool AppSettingPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}

	if (mIsEnable == FALSE) {
		return true;
	}

	if (mPrefix.IsEmpty()) {
		AfxMessageBox(_T("プレフィックスを入力してください"));
		return false;
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
			return false;
		}
	}

	return true;
}

bool AppSettingPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void AppSettingPage::OnOK()
{
	auto settingsPtr = mSettingsPtr;
	settingsPtr->Set(_T("ClipboardHistory:IsEnable"), (bool)mIsEnable);
	settingsPtr->Set(_T("ClipboardHistory:Prefix"), mPrefix);
	settingsPtr->Set(_T("ClipboardHistory:NumOfResults"), mNumOfResults);
	settingsPtr->Set(_T("ClipboardHistory:SizeLimit"), mSizeLimit);
	settingsPtr->Set(_T("ClipboardHistory:CountLimit"), mCountLimit);
	settingsPtr->Set(_T("ClipboardHistory:Interval"), mInterval);
	settingsPtr->Set(_T("ClipboardHistory:ExcludePattern"), mExcludePattern);

	__super::OnOK();
}

void AppSettingPage::DoDataExchange(CDataExchange* pDX)
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

BEGIN_MESSAGE_MAP(AppSettingPage, CDialog)
	ON_COMMAND(IDC_CHECK_ENABLE, OnUpdateStatus)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL AppSettingPage::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool AppSettingPage::UpdateStatus()
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

void AppSettingPage::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

	mIsEnable = settingsPtr->Get(_T("ClipboardHistory:IsEnable"), false);
	mPrefix = settingsPtr->Get(_T("ClipboardHistory:Prefix"), _T("cb"));
	mNumOfResults = settingsPtr->Get(_T("ClipboardHistory:NumOfResults"), 16);
	mSizeLimit = settingsPtr->Get(_T("ClipboardHistory:SizeLimit"), 4);
	mCountLimit = settingsPtr->Get(_T("ClipboardHistory:CountLimit"), 1024);
	mInterval = settingsPtr->Get(_T("ClipboardHistory:Interval"), 500);
	mExcludePattern = settingsPtr->Get(_T("ClipboardHistory:ExcludePattern"), _T(""));
}

void AppSettingPage::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageClipboardHistory::PImpl
{
	AppSettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageClipboardHistory)

AppSettingPageClipboardHistory::AppSettingPageClipboardHistory() : 
	AppSettingPageBase(_T("拡張機能"), _T("クリップボード履歴")),
	in(new PImpl)
{
}

AppSettingPageClipboardHistory::~AppSettingPageClipboardHistory()
{
}

// ウインドウを作成する
bool AppSettingPageClipboardHistory::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_CLIPBOARD, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageClipboardHistory::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageClipboardHistory::GetOrder()
{
	return 100;
}
// 
bool AppSettingPageClipboardHistory::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageClipboardHistory::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageClipboardHistory::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageClipboardHistory::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageClipboardHistory::GetHelpPageId(CString& id)
{
	id = _T("ClipboardHistorySetting");
	return true;
}


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

