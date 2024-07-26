// あ
#include "pch.h"
#include "framework.h"
#include "InputHistorySettingPage.h"
#include "setting/Settings.h"
#include "commands/common/ExecuteHistory.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

InputHistorySettingPage::InputHistorySettingPage(CWnd* parentWnd) : 
	SettingPage(_T("履歴"), IDD_APPSETTING_INPUTHISTORY, parentWnd),
	mIsUseHistory(false),
	mHistoryLimit(128)
{
}

InputHistorySettingPage::~InputHistorySettingPage()
{
}

BOOL InputHistorySettingPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL InputHistorySettingPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void InputHistorySettingPage::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("Input:IsUseHistory"), (bool)mIsUseHistory);
	settingsPtr->Set(_T("Input:HistoryLimit"), mHistoryLimit);

	__super::OnOK();
}

void InputHistorySettingPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_USEHISTORY, mIsUseHistory);
	DDX_Text(pDX, IDC_EDIT_COUNT, mHistoryLimit);
	DDV_MinMaxInt(pDX, mHistoryLimit, 1, 1024);
}

BEGIN_MESSAGE_MAP(InputHistorySettingPage, SettingPage)
	ON_COMMAND(IDC_BUTTON_CLEAR, OnButtonClear)
	ON_COMMAND(IDC_CHECK_USEHISTORY, OnUpdateData)
END_MESSAGE_MAP()


BOOL InputHistorySettingPage::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool InputHistorySettingPage::UpdateStatus()
{
	GetDlgItem(IDC_EDIT_COUNT)->EnableWindow(mIsUseHistory);
	GetDlgItem(IDC_BUTTON_CLEAR)->EnableWindow(mIsUseHistory);
	return true;
}

void InputHistorySettingPage::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();
	mIsUseHistory = settingsPtr->Get(_T("Input:IsUseHistory"), false);
	mHistoryLimit = settingsPtr->Get(_T("Input:HistoryLimit"), 128);
}

bool InputHistorySettingPage::GetHelpPageId(CString& id)
{
	id = _T("InputHistorySetting");
	return true;
}


void InputHistorySettingPage::OnButtonClear()
{
	auto history = launcherapp::commands::common::ExecuteHistory::GetInstance();
	history->ClearAllItems();
	history->Save();
}

void InputHistorySettingPage::OnUpdateData()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}
