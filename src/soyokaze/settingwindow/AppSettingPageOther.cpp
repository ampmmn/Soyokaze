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

struct AppSettingPageOther::PImpl
{
	// 長時間の連続稼働を警告する
	BOOL mIsWarnLongOperation = false;
	// 警告までの時間(分単位)
	int mTimeToWarnLongOperation = 90;
	// ログレベル
	int mLogLevel = 0;
};

AppSettingPageOther::AppSettingPageOther(CWnd* parentWnd) : 
	SettingPage(_T("その他"), IDD_APPSETTING_OTHER, parentWnd),
	in(std::make_unique<PImpl>())
{
}

AppSettingPageOther::~AppSettingPageOther()
{
}

BOOL AppSettingPageOther::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL AppSettingPageOther::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void AppSettingPageOther::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("Health:IsWarnLongOperation"), (bool)in->mIsWarnLongOperation);
	settingsPtr->Set(_T("Health:TimeToWarn"), in->mTimeToWarnLongOperation);

	// ログレベルを変換
	int spdLogLevel = 0;
	if (0 <= in->mLogLevel && in->mLogLevel < sizeof(SPDLOGLEVEL) / sizeof(*SPDLOGLEVEL)) {
		spdLogLevel = SPDLOGLEVEL[in->mLogLevel];
	}
	settingsPtr->Set(_T("Logging:Level"), spdLogLevel);

	__super::OnOK();
}

void AppSettingPageOther::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_WARNLONGWORKING, in->mIsWarnLongOperation);
	DDX_Text(pDX, IDC_EDIT_TIME, in->mTimeToWarnLongOperation);
	DDV_MinMaxInt(pDX, in->mTimeToWarnLongOperation, 1, 1440);
	DDX_CBIndex(pDX, IDC_COMBO_LEVEL, in->mLogLevel);
}

BEGIN_MESSAGE_MAP(AppSettingPageOther, SettingPage)
	ON_COMMAND(IDC_CHECK_WARNLONGWORKING, OnCheckWarnLongTime)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_LOGDIR, OnNotifyLinkOpen)
END_MESSAGE_MAP()


BOOL AppSettingPageOther::OnInitDialog()
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

bool AppSettingPageOther::UpdateStatus()
{
	GetDlgItem(IDC_EDIT_TIME)->EnableWindow(in->mIsWarnLongOperation);
	if (in->mIsWarnLongOperation == FALSE) {
		return true;
	}
	if (in->mIsWarnLongOperation && in->mTimeToWarnLongOperation <= 0) {
		AfxMessageBox(_T("警告までの時間は0より大きい値を指定してください"));
		return false;
	}
	return true;
}


void AppSettingPageOther::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();
	in->mIsWarnLongOperation = (BOOL)settingsPtr->Get(_T("Health:IsWarnLongOperation"), false);
	in->mTimeToWarnLongOperation = settingsPtr->Get(_T("Health:TimeToWarn"), 90);

	int spdLogLevel = settingsPtr->Get(_T("Logging:Level"), 6);
	for (int i = 0; i < sizeof(SPDLOGLEVEL) / sizeof(*SPDLOGLEVEL); ++i) {
		if (spdLogLevel != SPDLOGLEVEL[i]) {
			continue;
		}
		in->mLogLevel = i;
		break;
	}
}

bool AppSettingPageOther::GetHelpPageId(CString& id)
{
	id = _T("OtherSetting");
	return true;
}


void AppSettingPageOther::OnCheckWarnLongTime()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void AppSettingPageOther::OnNotifyLinkOpen(
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
