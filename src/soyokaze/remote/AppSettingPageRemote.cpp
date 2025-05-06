#include "pch.h"
#include "framework.h"
#include "AppSettingPageRemote.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class RemoteSettingDialog : public CDialog
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
	// サーバー機能
	BOOL mIsEnableServer{FALSE};
	// クライアント機能
	BOOL mIsEnableClient{FALSE};

	Settings* mSettingsPtr{nullptr};
};

void RemoteSettingDialog::OnOK()
{
	UpdateData();

	auto settingsPtr = mSettingsPtr;

	settingsPtr->Set(_T("Remote:IsEnableServer"), (bool)mIsEnableServer);
	settingsPtr->Set(_T("Remote:IsEnableClient"), (bool)mIsEnableClient);
	__super::OnOK();
}

void RemoteSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_ENABLE_REMOTESERVER, mIsEnableServer);
	DDX_Check(pDX, IDC_CHECK_ENABLE_REMOTECLIENT, mIsEnableClient);
	
}

BEGIN_MESSAGE_MAP(RemoteSettingDialog, CDialog)
	ON_COMMAND(IDC_CHECK_ENABLE_WINDOWTITLE, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLE_WORKSHEET, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLE_SLIDE, OnUpdateStatus)
END_MESSAGE_MAP()


BOOL RemoteSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool RemoteSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool RemoteSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void RemoteSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

	mIsEnableServer = settingsPtr->Get(_T("Remote:IsEnableServer"), false);
	mIsEnableClient = settingsPtr->Get(_T("Remote:IsEnableClient"), false);
}

void RemoteSettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
}


bool RemoteSettingDialog::UpdateStatus()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageRemote::PImpl
{
	RemoteSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageRemote)

AppSettingPageRemote::AppSettingPageRemote() : 
	AppSettingPageBase(_T("拡張機能"), _T("リモート")),
	in(new PImpl)
{
}

AppSettingPageRemote::~AppSettingPageRemote()
{
}

// ウインドウを作成する
bool AppSettingPageRemote::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_REMOTE, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageRemote::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageRemote::GetOrder()
{
	return 300;
}
// 
bool AppSettingPageRemote::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageRemote::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageRemote::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageRemote::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageRemote::GetHelpPageId(CString& id)
{
	id = _T("AppSettingSwitchWindow");
	return true;
}

