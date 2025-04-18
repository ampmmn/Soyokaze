// あ
#include "pch.h"
#include "framework.h"
#include "AppSettingPathConvertPage.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace pathconvert {

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

public:
	// git-bashパス変換を利用するか?
	BOOL mIsEnableGitBashPath{true};
	// file://プロトコル→ローカルパス変換を利用するか?
	BOOL mIsEnableFileProtolPath{true};

	Settings* mSettingsPtr{nullptr};
};

bool AppSettingPage::UpdateStatus()
{
	return true;
}

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
	auto settingsPtr = mSettingsPtr;
	settingsPtr->Set(_T("PathConvert:IsEnableGitBashPath"), (bool)mIsEnableGitBashPath);
	settingsPtr->Set(_T("PathConvert:IsEnableFileProtol"), (bool)mIsEnableFileProtolPath);

	__super::OnOK();
}

void AppSettingPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_ENABLE_GITBASH, mIsEnableGitBashPath);
	DDX_Check(pDX, IDC_CHECK_ENABLE_FILEPROTOCOL, mIsEnableFileProtolPath);
}

BEGIN_MESSAGE_MAP(AppSettingPage, CDialog)
END_MESSAGE_MAP()


BOOL AppSettingPage::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void AppSettingPage::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

	mIsEnableGitBashPath = settingsPtr->Get(_T("PathConvert:IsEnableGitBashPath"), true);
	mIsEnableFileProtolPath = settingsPtr->Get(_T("PathConvert:IsEnableFileProtol"), true);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPagePathConvert::PImpl
{
	AppSettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPagePathConvert)

AppSettingPagePathConvert::AppSettingPagePathConvert() : 
	AppSettingPageBase(_T("拡張機能"), _T("パス変換")),
	in(new PImpl)
{
}

AppSettingPagePathConvert::~AppSettingPagePathConvert()
{
}

// ウインドウを作成する
bool AppSettingPagePathConvert::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_PATHCONVERT, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPagePathConvert::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPagePathConvert::GetOrder()
{
	return 200;
}
// 
bool AppSettingPagePathConvert::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPagePathConvert::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPagePathConvert::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPagePathConvert::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPagePathConvert::GetHelpPageId(CString& id)
{
	id = _T("PathConvertSetting");
	return true;
}


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp

