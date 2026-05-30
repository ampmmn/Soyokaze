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


class InputHistorySettingPage : public CDialog
{
public:
	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	bool UpdateStatus();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonClear();
	afx_msg void OnUpdateData();

public:
	// 履歴機能を使う
	BOOL mIsUseHistory{FALSE};
	// 履歴件数
	int mHistoryLimit{128};

	Settings* mSettingsPtr{nullptr};
};

bool InputHistorySettingPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool InputHistorySettingPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void InputHistorySettingPage::OnOK()
{
	auto settingsPtr = mSettingsPtr;
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

BEGIN_MESSAGE_MAP(InputHistorySettingPage, CDialog)
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

void InputHistorySettingPage::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	mIsUseHistory = settingsPtr->Get(_T("Input:IsUseHistory"), false);
	mHistoryLimit = settingsPtr->Get(_T("Input:HistoryLimit"), 128);
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageInputHistory::PImpl
{
	InputHistorySettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageInputHistory)

AppSettingPageInputHistory::AppSettingPageInputHistory() : 
	AppSettingPageBase(_T("入力"), _T("履歴")),
	in(new PImpl)
{
}

AppSettingPageInputHistory::~AppSettingPageInputHistory()
{
}

// ウインドウを作成する
bool AppSettingPageInputHistory::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_INPUTHISTORY, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageInputHistory::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageInputHistory::GetOrder()
{
	return 30;
}
// 
bool AppSettingPageInputHistory::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageInputHistory::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageInputHistory::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageInputHistory::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageInputHistory::GetHelpPageId(String& id)
{
	id = "InputHistorySetting";
	return true;
}

