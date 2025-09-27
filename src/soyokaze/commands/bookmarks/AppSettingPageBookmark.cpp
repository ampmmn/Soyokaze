//あ
#include "pch.h"
#include "framework.h"
#include "AppSettingPageBookmark.h"
#include "commands/bookmarks/BookmarkCommandParam.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "gui\DDXWrapper.h"
#include "app/Manual.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace bookmarks {

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
	CommandParam mParam;
	Settings* mSettingsPtr{nullptr};
};

bool AppSettingPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
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
	mParam.Save(*mSettingsPtr);
	__super::OnOK();
}

void AppSettingPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_ENABLE, mParam.mIsEnable);
	DDX_Check(pDX, IDC_CHECK_ENABLE_CHROME, mParam.mIsEnableAltBrowser);
	DDX_Check(pDX, IDC_CHECK_ENABLE_EDGE, mParam.mIsEnableEdge);
	DDX_Check(pDX, IDC_CHECK_USEURL2, mParam.mIsUseURL);
	DDX_Text(pDX, IDC_EDIT_PREFIX, mParam.mPrefix);
	DDX_Text(pDX, IDC_EDIT_MINTRIGGERLENGTH, mParam.mMinTriggerLength);
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
	bool isEnable = mParam.mIsEnable;

	auto checkUseOther = GetDlgItem(IDC_CHECK_ENABLE_CHROME);
	ASSERT(checkUseOther);

	// Webブラウザ(外部ツール)の表記を変える
	if (mSettingsPtr) {
		CString otherBrowserName;

		bool useChrome = mSettingsPtr->Get(_T("ExternalTool:BrowserUseChrome"), true);
		if (useChrome) {
			otherBrowserName = _T("Chrome");
		}
		else {
			otherBrowserName = _T("Webブラウザ(外部ツール)");
		}
		checkUseOther->SetWindowText(fmt::format(_T("{}のブックマークを候補として表示する(&C)"), (LPCTSTR)otherBrowserName).c_str());
	}

	// 機能を無効にする場合はコントロールをグレーアウトする
	checkUseOther->EnableWindow(isEnable);
	GetDlgItem(IDC_CHECK_ENABLE_EDGE)->EnableWindow(isEnable);
	GetDlgItem(IDC_CHECK_USEURL2)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_PREFIX)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_MINTRIGGERLENGTH)->EnableWindow(isEnable);

	return true;
}

void AppSettingPage::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	mParam.Load(*mSettingsPtr);
}

void AppSettingPage::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageBookmark::PImpl
{
	AppSettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageBookmark)

AppSettingPageBookmark::AppSettingPageBookmark() : 
	AppSettingPageBase(_T("拡張機能"), _T("ブックマーク検索")),
	in(new PImpl)
{
}

AppSettingPageBookmark::~AppSettingPageBookmark()
{
}

// ウインドウを作成する
bool AppSettingPageBookmark::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_BOOKMARK, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageBookmark::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageBookmark::GetOrder()
{
	return 230;
}
// 
bool AppSettingPageBookmark::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageBookmark::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageBookmark::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageBookmark::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageBookmark::GetHelpPageId(String& id)
{
	id = "BookmarkSetting";
	return true;
}

}}} // end of namespace launcherapp::commands::bookmarks

