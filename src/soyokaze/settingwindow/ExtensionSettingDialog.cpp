#include "pch.h"
#include "framework.h"
#include "ExtensionSettingDialog.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class ExtensionSettingDialog : public CDialog
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
	afx_msg void OnBrowsePyhonDLLPath();
	afx_msg void OnUpdateStatus();

public:
	BOOL mIsEnableCalc{FALSE};
	CString mPythonDLLPath;

	// コントロールパネル選択機能
	BOOL mIsEnableControlPanel{FALSE};
	// スタートメニュー/最近使ったファイル選択機能
	BOOL mIsEnableSpecialFolder{FALSE};
	// UWPアプリ選択機能
	BOOL mIsEnableUWP{FALSE};
	// MMCスナップイン選択機能
	BOOL mIsEnableMMCSnapin{FALSE};
	// Windowsの設定(ms-settings)選択機能
	BOOL mIsEnableMSSettings{FALSE};
	// 環境変数選択機能
	BOOL mIsEnableEnvironment{FALSE};
	// 環境変数のプレフィックス
	CString mPrefixEnvironment;

	Settings* mSettingsPtr{nullptr};
};

void ExtensionSettingDialog::OnOK()
{
	UpdateData();

	auto settingsPtr = mSettingsPtr;

	settingsPtr->Set(_T("Calculator:Enable"), (bool)mIsEnableCalc);
	settingsPtr->Set(_T("Soyokaze:PythonDLLPath"), mPythonDLLPath);
	settingsPtr->Set(_T("Soyokaze:IsEnableControlPanel"), (bool)mIsEnableControlPanel);
	settingsPtr->Set(_T("Soyokaze:IsEnableSpecialFolder"), (bool)mIsEnableSpecialFolder);
	settingsPtr->Set(_T("Soyokaze:IsEnableUWP"), (bool)mIsEnableUWP);
	settingsPtr->Set(_T("Soyokaze:IsEnableMMCSnapin"), (bool)mIsEnableMMCSnapin);
	settingsPtr->Set(_T("Soyokaze:IsEnableMSSettings"), (bool)mIsEnableMSSettings);
	settingsPtr->Set(_T("Environment:IsEnable"), (bool)mIsEnableEnvironment);
	settingsPtr->Set(_T("Environment:Prefix"), mPrefixEnvironment);

	__super::OnOK();
}

void ExtensionSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_ENABLECALCULATOR, mIsEnableCalc);
	DDX_Text(pDX, IDC_EDIT_PYTHONDLLPATH, mPythonDLLPath);
	DDX_Check(pDX, IDC_CHECK_ENABLE_CONTROLPANEL, mIsEnableControlPanel);
	DDX_Check(pDX, IDC_CHECK_ENABLE_SPECIALFOLDER, mIsEnableSpecialFolder);
	DDX_Check(pDX, IDC_CHECK_ENABLE_UWPAPPS, mIsEnableUWP);
	DDX_Check(pDX, IDC_CHECK_ENABLE_MMCSNAPINS, mIsEnableMMCSnapin);
	DDX_Check(pDX, IDC_CHECK_ENABLE_MSSETTINGS, mIsEnableMSSettings);
	DDX_Check(pDX, IDC_CHECK_ENABLE_ENVIRONMENT, mIsEnableEnvironment);
	DDX_Text(pDX, IDC_EDIT_PREFIX_ENVIRONMENT, mPrefixEnvironment);
}

BEGIN_MESSAGE_MAP(ExtensionSettingDialog, CDialog)
	ON_COMMAND(IDC_BUTTON_BROWSE, OnBrowsePyhonDLLPath)
	ON_COMMAND(IDC_CHECK_ENABLECALCULATOR, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLE_ENVIRONMENT, OnUpdateStatus)
END_MESSAGE_MAP()


BOOL ExtensionSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ExtensionSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	if (mIsEnableCalc && Path::FileExists(mPythonDLLPath) == FALSE) {
		AfxMessageBox(_T("Python(DLL)のパスを設定してください\n(ファイルが存在しません)"));
		return false;
	}
	if (mPrefixEnvironment.IsEmpty()) {
		AfxMessageBox(_T("環境変数のプレフィックスを入力してください"));
		return false;
	}

	return true;
}

bool ExtensionSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void ExtensionSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

	mIsEnableCalc = settingsPtr->Get(_T("Calculator:Enable"), false);
	mPythonDLLPath = settingsPtr->Get(_T("Soyokaze:PythonDLLPath"), _T(""));

	mIsEnableControlPanel = settingsPtr->Get(_T("Soyokaze:IsEnableControlPanel"), true);
	mIsEnableSpecialFolder = settingsPtr->Get(_T("Soyokaze:IsEnableSpecialFolder"), true);
	mIsEnableUWP = settingsPtr->Get(_T("Soyokaze:IsEnableUWP"), true);
	mIsEnableMMCSnapin = settingsPtr->Get(_T("Soyokaze:IsEnableMMCSnapin"), true);
	mIsEnableMSSettings = settingsPtr->Get(_T("Soyokaze:IsEnableMSSettings"), true);
	mIsEnableEnvironment = settingsPtr->Get(_T("Environment:IsEnable"), true);
	mPrefixEnvironment = settingsPtr->Get(_T("Environment:Prefix"), _T("env"));
}

bool ExtensionSettingDialog::UpdateStatus()
{
	GetDlgItem(IDC_EDIT_PYTHONDLLPATH)->EnableWindow(mIsEnableCalc);
	GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(mIsEnableCalc);

	GetDlgItem(IDC_EDIT_PREFIX_ENVIRONMENT)->EnableWindow(mIsEnableEnvironment);

	return true;
}

void ExtensionSettingDialog::OnBrowsePyhonDLLPath()
{
	UpdateData();

	CString filterStr((LPCTSTR)IDS_FILTER_DLL);
	CFileDialog dlg(TRUE, NULL, mPythonDLLPath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mPythonDLLPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void ExtensionSettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageExtension::PImpl
{
	ExtensionSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageExtension)

AppSettingPageExtension::AppSettingPageExtension() : 
	AppSettingPageBase(_T(""), _T("拡張機能")),
	in(new PImpl)
{
}

AppSettingPageExtension::~AppSettingPageExtension()
{
}

// ウインドウを作成する
bool AppSettingPageExtension::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_EXTENSIONSETTING, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageExtension::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageExtension::GetOrder()
{
	return 90;
}
// 
bool AppSettingPageExtension::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageExtension::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageExtension::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageExtension::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageExtension::GetHelpPageId(CString& id)
{
	id = _T("ExtensionSetting");
	return true;
}

