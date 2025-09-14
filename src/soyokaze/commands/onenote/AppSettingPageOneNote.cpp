//あ
#include "pch.h"
#include "framework.h"
#include "AppSettingPageOneNote.h"
#include "commands/onenote/OneNoteCommandParam.h"
#include "setting/Settings.h"
#include "gui\DDXWrapper.h"
#include "app/Manual.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace onenote {

class AppSettingPage : public CDialog
{
public:
	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	void OnOK() override;
protected:
	bool UpdateStatus();

	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();

public:
	CommandParam mParam;
	Settings* mSettingsPtr{nullptr};
};

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
	mParam.Save(*mSettingsPtr);
	__super::OnOK();
}

void AppSettingPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_ENABLE, mParam.mIsEnable);
	DDX_Text(pDX, IDC_EDIT_PREFIX, mParam.mPrefix);
	DDX_Text(pDX, IDC_EDIT_MINTRIGGERLENGTH, mParam.mMinTriggerLength);
	DDV_MinMaxInt(pDX,  mParam.mMinTriggerLength, 0, 32);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AppSettingPage, CDialog)
	ON_COMMAND(IDC_CHECK_ENABLE, OnUpdateStatus)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL AppSettingPage::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool AppSettingPage::UpdateStatus()
{
	bool isEnable = mParam.mIsEnable;

	GetDlgItem(IDC_EDIT_PREFIX)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_MINTRIGGERLENGTH)->EnableWindow(isEnable);

	return true;
}

void AppSettingPage::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	mParam.Load(*mSettingsPtr);
}

void AppSettingPage::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct AppSettingPageOneNote::PImpl
{
	AppSettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageOneNote)

AppSettingPageOneNote::AppSettingPageOneNote() : 
	AppSettingPageBase(_T("拡張機能"), _T("OneNote")),
	in(new PImpl)
{
}

AppSettingPageOneNote::~AppSettingPageOneNote()
{
}

// ウインドウを作成する
bool AppSettingPageOneNote::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_ONENOTE, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageOneNote::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageOneNote::GetOrder()
{
	return 210;
}

// 
bool AppSettingPageOneNote::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageOneNote::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageOneNote::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageOneNote::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageOneNote::GetHelpPageId(String& id)
{
	id = "OneNoteSetting";
	return true;
}





}}} // end of namespace launcherapp::commands::onenote

