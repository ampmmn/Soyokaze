//あ
#include "pch.h"
#include "framework.h"
#include "AppSettingEverythingPage.h"
#include "commands/everything/EverythingCommandParam.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "gui\DDXWrapper.h"
#include "app/Manual.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace everything {

class AppSettingPage : public CDialog
{
public:
	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	void OnOK() override;
protected:
	bool UpdateStatus();

	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg void OnButtonBrowse();
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);

public:
	CommandParam mParam;
	Settings* mSettingsPtr{nullptr};
};

bool AppSettingPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}

	if (mParam.mIsRunApp && Path::FileExists(mParam.mEverythingExePath) == FALSE) {
		CString msg;
		msg.Format(_T("%sは有効なパスではありません"), (LPCTSTR)mParam.mEverythingExePath);
		AfxMessageBox(msg);
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
	DDX_Check(pDX, IDC_CHECK_RUNEVERYTHING, mParam.mIsRunApp);
	DDX_Text(pDX, IDC_EDIT_EVERYTHINGEXEPATH, mParam.mEverythingExePath);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AppSettingPage, CDialog)
	ON_COMMAND(IDC_CHECK_ENABLE, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_RUNEVERYTHING, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE1, OnButtonBrowse)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_COMMAND, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_COMMAND, OnNotifyLinkOpen)
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
	BOOL isRunApp = mParam.mIsRunApp ? TRUE : FALSE;

	GetDlgItem(IDC_EDIT_PREFIX)->EnableWindow(isEnable);
	GetDlgItem(IDC_CHECK_RUNEVERYTHING)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_EVERYTHINGEXEPATH)->EnableWindow(isEnable && isRunApp);
	GetDlgItem(IDC_BUTTON_BROWSEFILE1)->EnableWindow(isEnable && isRunApp);

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

void AppSettingPage::OnButtonBrowse()
{
	UpdateData();

	CString filterStr(_T("Everything.exe|Everything.exe|実行ファイル(*.exe)|*.exe||"));
	CFileDialog dlg(TRUE, NULL, mParam.mEverythingExePath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mEverythingExePath = dlg.GetPathName();
	UpdateData(FALSE);
}

// マニュアル表示
void AppSettingPage::OnNotifyLinkOpen(
	NMHDR *,
 	LRESULT *pResult
)
{
	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("EverythingEdit"));
	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct AppSettingPageEverything::PImpl
{
	AppSettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageEverything)

AppSettingPageEverything::AppSettingPageEverything() : 
	AppSettingPageBase(_T("拡張機能"), _T("Everything")),
	in(new PImpl)
{
}

AppSettingPageEverything::~AppSettingPageEverything()
{
}

// ウインドウを作成する
bool AppSettingPageEverything::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_EVERYTHING, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageEverything::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageEverything::GetOrder()
{
	return 250;
}
// 
bool AppSettingPageEverything::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageEverything::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageEverything::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageEverything::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageEverything::GetHelpPageId(CString& id)
{
	id = _T("EverythingSetting");
	return true;
}





} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

