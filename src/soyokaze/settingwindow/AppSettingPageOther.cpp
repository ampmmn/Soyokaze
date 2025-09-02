#include "pch.h"
#include "framework.h"
#include "AppSettingPageOther.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "setting/Settings.h"
#include "logger/Logger.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;


static int SPDLOGLEVEL[] = { 
	6,   // Off
	4,   // Error
	3,   // Warn
	2,   // Info
	1,   // Debug
};

// 
class OtherSettingDialog : public CDialog
{
public:
	bool UpdateStatus();

	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnTransparencyChanged();
	afx_msg void OnCheckWarnLongTime();
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);

	// 長時間の連続稼働を警告する
	BOOL mIsWarnLongOperation{FALSE};
	// 警告までの時間(分単位)
	int mTimeToWarnLongOperation{90};
	// ログレベル
	int mLogLevel{0};
	// 性能ログを出力する
	BOOL mIsEnablePerfLog{FALSE};

	Settings* mSettingsPtr{nullptr};
};

bool OtherSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool OtherSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void OtherSettingDialog::OnOK()
{
	if (UpdateData() == FALSE) {
		return;
	}

	auto settingsPtr = mSettingsPtr;
	settingsPtr->Set(_T("Health:IsWarnLongOperation"), (bool)mIsWarnLongOperation);
	settingsPtr->Set(_T("Health:TimeToWarn"), mTimeToWarnLongOperation);

	// ログレベルを変換
	int spdLogLevel = 0;
	if (0 <= mLogLevel && mLogLevel < sizeof(SPDLOGLEVEL) / sizeof(*SPDLOGLEVEL)) {
		spdLogLevel = SPDLOGLEVEL[mLogLevel];
	}
	settingsPtr->Set(_T("Logging:Level"), spdLogLevel);
	settingsPtr->Set(_T("Logging:UsePerformanceLog"), mIsEnablePerfLog != FALSE);

	__super::OnOK();
}

void OtherSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_WARNLONGWORKING, mIsWarnLongOperation);
	DDX_Text(pDX, IDC_EDIT_TIME, mTimeToWarnLongOperation);
	DDV_MinMaxInt(pDX, mTimeToWarnLongOperation, 1, 1440);
	DDX_CBIndex(pDX, IDC_COMBO_LEVEL, mLogLevel);
	DDX_Check(pDX, IDC_CHECK_ENABLEPERFLOG, mIsEnablePerfLog);
}

BEGIN_MESSAGE_MAP(OtherSettingDialog, CDialog)
	ON_COMMAND(IDC_CHECK_WARNLONGWORKING, OnCheckWarnLongTime)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_LOGDIR, OnNotifyLinkOpen)
END_MESSAGE_MAP()


BOOL OtherSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	CWnd* link = GetDlgItem(IDC_SYSLINK_LOGDIR);
	CString str;
	link->GetWindowText(str);

	auto logDir = Logger::Get()->GetLogDirectory();
	str.Replace(_T("$LOGDIR"), logDir);
	link->SetWindowText(str);

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool OtherSettingDialog::UpdateStatus()
{
	GetDlgItem(IDC_EDIT_TIME)->EnableWindow(mIsWarnLongOperation);
	if (mIsWarnLongOperation == FALSE) {
		return true;
	}
	if (mIsWarnLongOperation && mTimeToWarnLongOperation <= 0) {
		AfxMessageBox(_T("警告までの時間は0より大きい値を指定してください"));
		return false;
	}
	return true;
}


void OtherSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	mIsWarnLongOperation = (BOOL)settingsPtr->Get(_T("Health:IsWarnLongOperation"), false);
	mTimeToWarnLongOperation = settingsPtr->Get(_T("Health:TimeToWarn"), 90);

	int spdLogLevel = settingsPtr->Get(_T("Logging:Level"), 6);
	for (int i = 0; i < sizeof(SPDLOGLEVEL) / sizeof(*SPDLOGLEVEL); ++i) {
		if (spdLogLevel != SPDLOGLEVEL[i]) {
			continue;
		}
		mLogLevel = i;
		break;
	}
	mIsEnablePerfLog = settingsPtr->Get(_T("Logging:UsePerformanceLog"), false);
}


void OtherSettingDialog::OnCheckWarnLongTime()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void OtherSettingDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	NMLINK* linkPtr = (NMLINK*)pNMHDR;

	if (linkPtr->hdr.idFrom == IDC_SYSLINK_LOGDIR) {
		// ログ出力ディレクトリを開く
		auto logDir = Logger::Get()->GetLogDirectory();
		logDir += _T("\\");

		ShellExecCommand cmd;
		cmd.SetPath((LPCTSTR)logDir);
		cmd.Execute(CommandParameterBuilder::EmptyParam());
	}
	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageOther::PImpl
{
	OtherSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageOther)

AppSettingPageOther::AppSettingPageOther() : 
	AppSettingPageBase(_T(""), _T("その他")),
	in(new PImpl)
{
}

AppSettingPageOther::~AppSettingPageOther()
{
}

// ウインドウを作成する
bool AppSettingPageOther::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_OTHER, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageOther::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageOther::GetOrder()
{
	return 110;
}
// 
bool AppSettingPageOther::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageOther::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageOther::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageOther::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageOther::GetHelpPageId(String& id)
{
	id = "OtherSetting";
	return true;
}

