//あ
#include "pch.h"
#include "framework.h"
#include "AppSettingPageUIAutomation.h"
#include "commands/uiautomation/UIAutomationParam.h"
#include "setting/Settings.h"
#include "control\DDXWrapper.h"
#include "app/Manual.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace uiautomation {

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
	DDX_Text(pDX, IDC_EDIT_PREFIX, mParam.mPrefix);
	DDX_Check(pDX, IDC_CHECK_ENABLEMENUITEM, mParam.mIsEnableMenuItem);
	DDX_Check(pDX, IDC_CHECK_ENABLEPROPPAGES, mParam.mIsEnableTabPages);
	DDX_Check(pDX, IDC_CHECK_DEBUGDUMPENABLED, mParam.mIsDebugDumpEnabled);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AppSettingPage, CDialog)
	ON_COMMAND(IDC_CHECK_ENABLE, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLEMENUITEM, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLEPROPPAGES, OnUpdateStatus)
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

	GetDlgItem(IDC_EDIT_PREFIX)->EnableWindow(isEnable);
	GetDlgItem(IDC_CHECK_ENABLEMENUITEM)->EnableWindow(isEnable);
	GetDlgItem(IDC_CHECK_ENABLEPROPPAGES)->EnableWindow(isEnable);
	GetDlgItem(IDC_CHECK_DEBUGDUMPENABLED)->EnableWindow(isEnable);
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


struct AppSettingPageUIAutomation::PImpl
{
	AppSettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageUIAutomation)

AppSettingPageUIAutomation::AppSettingPageUIAutomation() : 
	AppSettingPageBase(_T("拡張機能"), _T("UI要素")),
	in(new PImpl)
{
}

AppSettingPageUIAutomation::~AppSettingPageUIAutomation()
{
}

// ウインドウを作成する
bool AppSettingPageUIAutomation::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_UIAUTOMATION, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageUIAutomation::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageUIAutomation::GetOrder()
{
	return 290;
}
// 
bool AppSettingPageUIAutomation::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageUIAutomation::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageUIAutomation::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageUIAutomation::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageUIAutomation::GetHelpPageId(String& id)
{
	id = "UIAutomationSetting";
	return true;
}

}}} // end of namespace launcherapp::commands::uiautomation
