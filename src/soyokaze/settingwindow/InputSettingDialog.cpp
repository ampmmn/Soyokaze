#include "pch.h"
#include "framework.h"
#include "InputSettingDialog.h"
#include "commands/builtin/MainDirCommand.h"
#include "actions/core/ActionParameter.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "externaltool/webbrowser/ConfiguredBrowserEnvironment.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using MainDirCommand = launcherapp::commands::builtin::MainDirCommand;
using ConfiguredBrowserEnvironment = launcherapp::externaltool::webbrowser::ConfiguredBrowserEnvironment;

class InputSettingDialog : public CDialog
{
public:
	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	// 入力画面を表示するときにIMEをオフにする
	BOOL mIsIMEOff{FALSE};

	// ネットワークパスを無視する
	BOOL mIsIgnoreUNC{FALSE};

	// C/Migemo検索を有効にする
	BOOL mIsEnableMigemo{TRUE};

	Settings* mSettingsPtr{nullptr};

	bool UpdateStatus();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
};



bool InputSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool InputSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void InputSettingDialog::OnOK()
{
	auto settingsPtr = mSettingsPtr;
	settingsPtr->Set(_T("Soyokaze:IsIMEOffOnActive"), (bool)mIsIMEOff);
	settingsPtr->Set(_T("Soyokaze:IsIgnoreUNC"), (bool)mIsIgnoreUNC);
	settingsPtr->Set(_T("Soyokaze:IsEnableMigemo"), (bool)mIsEnableMigemo);

	__super::OnOK();
}

void InputSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_IMEOFF, mIsIMEOff);
	DDX_Check(pDX, IDC_CHECK_IGNOREUNC, mIsIgnoreUNC);
	DDX_Check(pDX, IDC_CHECK_ENABLEMIGEMO, mIsEnableMigemo);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(InputSettingDialog, CDialog)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, OnNotifyLinkOpen)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_APPDIR, OnNotifyLinkOpen)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL InputSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	CWnd* link2 = GetDlgItem(IDC_SYSLINK_APPDIR);
	ASSERT(link2);
	CString str;
	link2->GetWindowText(str);

	Path appDir(Path::MODULEFILEDIR);
	str.Replace(_T("$APPDIR"), appDir);
	link2->SetWindowText(str);

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool InputSettingDialog::UpdateStatus()
{
	return true;
}

void InputSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	mIsIMEOff = settingsPtr->Get(_T("Soyokaze:IsIMEOffOnActive"), false);
	mIsIgnoreUNC = settingsPtr->Get(_T("Soyokaze:IsIgnoreUNC"), false);
	mIsEnableMigemo = settingsPtr->Get(_T("Soyokaze:IsEnableMigemo"), true);
}


void InputSettingDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	NMLINK* linkPtr = (NMLINK*)pNMHDR;

	if (linkPtr->hdr.idFrom == IDC_SYSLINK_APPDIR) {
		auto param = launcherapp::actions::core::ParameterBuilder::Create();
		MainDirCommand cmd(_T("tmp"));
		cmd.Execute(param);

		param->Release();
	}
	else {
		// アプリ設定の 外部ツール > Webブラウザ の設定でURLを開く
		ConfiguredBrowserEnvironment::GetInstance()->OpenURL(linkPtr->item.szUrl);
	}
	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageInput::PImpl
{
	InputSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageInput)

AppSettingPageInput::AppSettingPageInput() : 
	AppSettingPageBase(_T(""), _T("入力")),
	in(new PImpl)
{
}

AppSettingPageInput::~AppSettingPageInput()
{
}

// ウインドウを作成する
bool AppSettingPageInput::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_INPUTSETTING, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageInput::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageInput::GetOrder()
{
	return 30;
}
// 
bool AppSettingPageInput::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageInput::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageInput::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageInput::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageInput::GetHelpPageId(String& id)
{
	id = "InputSetting";
	return true;
}

