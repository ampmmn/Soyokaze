//あ
#include "pch.h"
#include "framework.h"
#include "AppSettingWinScpPage.h"
#include "commands/winscp/WinScpCommandParam.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "gui\DDXWrapper.h"
#include "app/Manual.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace winscp {

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
	DDX_Check(pDX, IDC_CHECK_USEPORTABLE, mParam.mIsUsePortable);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, mParam.mWinScpExeFilePath);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AppSettingPage, CDialog)
	ON_COMMAND(IDC_CHECK_ENABLE, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_USEPORTABLE, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSE, OnButtonBrowseFile)
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
	bool isUsePortable = mParam.mIsUsePortable;

	GetDlgItem(IDC_CHECK_USEPORTABLE)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_FILEPATH)->EnableWindow(isEnable && isUsePortable);
	GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(isEnable && isUsePortable);
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

void AppSettingPage::OnButtonBrowseFile()
{
	UpdateData();

	CString filterStr((LPCTSTR)IDS_FILTER_EXE);
	CFileDialog dlg(TRUE, NULL, mParam.mWinScpExeFilePath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mWinScpExeFilePath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageWinScp::PImpl
{
	AppSettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageWinScp)

AppSettingPageWinScp::AppSettingPageWinScp() : 
	AppSettingPageBase(_T("拡張機能"), _T("WinSCP")),
	in(new PImpl)
{
}

AppSettingPageWinScp::~AppSettingPageWinScp()
{
}

// ウインドウを作成する
bool AppSettingPageWinScp::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_WINSCP, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageWinScp::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageWinScp::GetOrder()
{
	return 300;
}
// 
bool AppSettingPageWinScp::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageWinScp::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageWinScp::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageWinScp::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageWinScp::GetHelpPageId(CString& id)
{
	id = _T("WinScpSetting");
	return true;
}

}}} // end of namespace launcherapp::commands::winscp

