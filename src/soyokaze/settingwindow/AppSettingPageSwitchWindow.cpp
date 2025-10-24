#include "pch.h"
#include "framework.h"
#include "AppSettingPageSwitchWindow.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class SwitchWindowSettingDialog : public CDialog
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
	// ウインドウタイトルによる切り替えを発動するためのプレフィックス
	CString mPrefixWindowTitle;
	// ワークシート名による切り替えを発動するためのプレフィックス
	CString mPrefixWorksheet;
	// Powerpointスライド名による切り替えを発動するためのプレフィックス
	CString mPrefixPresentation;
	// Outlookのフォルダ名による切り替えを発動するためのプレフィックス
	CString mPrefixOutlook;
	// ウインドウタイトルによるウインドウ切り替え機能
	BOOL mIsEnableWindowTitle{FALSE};
	// Excelワークシート名によるウインドウ切り替え機能
	BOOL mIsEnableWorksheet{FALSE};
	// PowerPointスライド名によるウインドウ切り替え機能
	BOOL mIsEnableSlide{FALSE};
	// Outlookフォルダ名によるウインドウ切り替え機能
	BOOL mIsEnableOutlook{FALSE};

	Settings* mSettingsPtr{nullptr};
};

void SwitchWindowSettingDialog::OnOK()
{
	UpdateData();

	auto settingsPtr = mSettingsPtr;

	settingsPtr->Set(_T("WindowSwitch:Prefix"), mPrefixWindowTitle);
	settingsPtr->Set(_T("WindowSwitch:EnableWindowSwitch"), (bool)mIsEnableWindowTitle);
	settingsPtr->Set(_T("Excel:Prefix"), mPrefixWorksheet);
	settingsPtr->Set(_T("Excel:EnableWorkSheet"), (bool)mIsEnableWorksheet);
	settingsPtr->Set(_T("PowerPoint:Prefix"), mPrefixPresentation);
	settingsPtr->Set(_T("PowerPoint:EnableSlide"), (bool)mIsEnableSlide);
	settingsPtr->Set(_T("Outlook:Prefix"), mPrefixOutlook);
	settingsPtr->Set(_T("Outlook:EnableFolderName"), (bool)mIsEnableOutlook);
	__super::OnOK();
}

void SwitchWindowSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PREFIX_WINDOWTITLE, mPrefixWindowTitle);
	DDX_Check(pDX, IDC_CHECK_ENABLE_WINDOWTITLE, mIsEnableWindowTitle);
	DDX_Text(pDX, IDC_EDIT_PREFIX_WORKSHEET, mPrefixWorksheet);
	DDX_Check(pDX, IDC_CHECK_ENABLE_WORKSHEET, mIsEnableWorksheet);
	DDX_Text(pDX, IDC_EDIT_PREFIX_PRESENTATION, mPrefixPresentation);
	DDX_Check(pDX, IDC_CHECK_ENABLE_SLIDE, mIsEnableSlide);
	DDX_Text(pDX, IDC_EDIT_PREFIX_OUTLOOK, mPrefixOutlook);
	DDX_Check(pDX, IDC_CHECK_ENABLE_OUTLOOK, mIsEnableOutlook);
	
}

BEGIN_MESSAGE_MAP(SwitchWindowSettingDialog, CDialog)
	ON_COMMAND(IDC_CHECK_ENABLE_WINDOWTITLE, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLE_WORKSHEET, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLE_SLIDE, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLE_OUTLOOK, OnUpdateStatus)
END_MESSAGE_MAP()


BOOL SwitchWindowSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool SwitchWindowSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	if (mPrefixWindowTitle.IsEmpty()) {
		AfxMessageBox(_T("プレフィックスを入力してください"));
		return false;
	}
	return true;
}

bool SwitchWindowSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void SwitchWindowSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

	mPrefixWindowTitle = settingsPtr->Get(_T("WindowSwitch:Prefix"), _T("wl"));
	mIsEnableWindowTitle = settingsPtr->Get(_T("WindowSwitch:EnableWindowSwitch"), true);
	mPrefixWorksheet = settingsPtr->Get(_T("Excel:Prefix"), _T("xj"));
	mIsEnableWorksheet = settingsPtr->Get(_T("Excel:EnableWorkSheet"), true);
	mPrefixPresentation = settingsPtr->Get(_T("PowerPoint:Prefix"), _T("pj"));
	mIsEnableSlide = settingsPtr->Get(_T("PowerPoint:EnableSlide"), false);
	mPrefixOutlook = settingsPtr->Get(_T("Outlook:Prefix"), _T("ol"));
	mIsEnableOutlook = settingsPtr->Get(_T("Outlook:EnableFolderName"), false);
}

void SwitchWindowSettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
}


bool SwitchWindowSettingDialog::UpdateStatus()
{
	GetDlgItem(IDC_EDIT_PREFIX_WINDOWTITLE)->EnableWindow(mIsEnableWindowTitle != 0);
	GetDlgItem(IDC_EDIT_PREFIX_WORKSHEET)->EnableWindow(mIsEnableWorksheet != 0);
	GetDlgItem(IDC_EDIT_PREFIX_PRESENTATION)->EnableWindow(mIsEnableSlide != 0);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageSwitchWindow::PImpl
{
	SwitchWindowSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageSwitchWindow)

AppSettingPageSwitchWindow::AppSettingPageSwitchWindow() : 
	AppSettingPageBase(_T("拡張機能"), _T("ウインドウ切替")),
	in(new PImpl)
{
}

AppSettingPageSwitchWindow::~AppSettingPageSwitchWindow()
{
}

// ウインドウを作成する
bool AppSettingPageSwitchWindow::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_SWITCHWINDOW, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageSwitchWindow::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageSwitchWindow::GetOrder()
{
	return 50;
}
// 
bool AppSettingPageSwitchWindow::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageSwitchWindow::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageSwitchWindow::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageSwitchWindow::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageSwitchWindow::GetHelpPageId(String& id)
{
	id = "AppSettingSwitchWindow";
	return true;
}

