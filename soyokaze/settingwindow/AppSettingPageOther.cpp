#include "pch.h"
#include "framework.h"
#include "AppSettingPageOther.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct AppSettingPageOther::PImpl
{
	// 長時間の連続稼働を警告する
	BOOL mIsWarnLongOperation;
	// 警告までの時間(分単位)
	int mTimeToWarnLongOperation;
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
	__super::OnOK();
}

void AppSettingPageOther::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_WARNLONGWORKING, in->mIsWarnLongOperation);
	DDX_Text(pDX, IDC_EDIT_TIME, in->mTimeToWarnLongOperation);
	DDV_MinMaxInt(pDX, in->mTimeToWarnLongOperation, 1, 1440);
}

BEGIN_MESSAGE_MAP(AppSettingPageOther, SettingPage)
	ON_COMMAND(IDC_CHECK_WARNLONGWORKING, OnCheckWarnLongTime)
END_MESSAGE_MAP()


BOOL AppSettingPageOther::OnInitDialog()
{
	__super::OnInitDialog();

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
}

void AppSettingPageOther::OnCheckWarnLongTime()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

