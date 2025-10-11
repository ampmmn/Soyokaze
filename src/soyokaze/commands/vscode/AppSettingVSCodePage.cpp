//あ
#include "pch.h"
#include "framework.h"
#include "AppSettingVSCodePage.h"
#include "commands/vscode/VSCodeCommandParam.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "gui\DDXWrapper.h"
#include "app/Manual.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace vscode {

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
	afx_msg void OnButtonBrowseFile();

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
	DDX_Check(pDX, IDC_CHECK_FULLPATH, mParam.mIsShowFullPath);
	DDX_Text(pDX, IDC_EDIT_PREFIX, mParam.mPrefix);
	DDX_Text(pDX, IDC_EDIT_MINTRIGGERLENGTH, mParam.mMinTriggerLength);
	DDV_MinMaxInt(pDX,  mParam.mMinTriggerLength, 0, 32);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AppSettingPage, CDialog)
	ON_COMMAND(IDC_CHECK_ENABLE, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_FULLPATH, OnUpdateStatus)
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

	GetDlgItem(IDC_CHECK_FULLPATH)->EnableWindow(isEnable);
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


struct AppSettingPageVSCode::PImpl
{
	AppSettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageVSCode)

AppSettingPageVSCode::AppSettingPageVSCode() : 
	AppSettingPageBase(_T("拡張機能"), _T("Visual Studio Code")),
	in(new PImpl)
{
}

AppSettingPageVSCode::~AppSettingPageVSCode()
{
}

// ウインドウを作成する
bool AppSettingPageVSCode::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_VSCODE, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageVSCode::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageVSCode::GetOrder()
{
	return 400;
}
// 
bool AppSettingPageVSCode::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageVSCode::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageVSCode::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageVSCode::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageVSCode::GetHelpPageId(String& id)
{
	id = "VSCodeSetting";
	return true;
}

}}} // end of namespace launcherapp::commands::vscode

