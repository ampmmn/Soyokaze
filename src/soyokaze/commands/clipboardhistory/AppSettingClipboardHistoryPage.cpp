//あ
#include "pch.h"
#include "framework.h"
#include "AppSettingClipboardHistoryPage.h"
#include "commands/clipboardhistory/ClipboardHistoryParam.h"
#include "setting/Settings.h"
#include "utility/Path.h"
#include "control/DDXWrapper.h"
#include "app/Manual.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

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
	afx_msg void OnUpdateStatus();

public:
	Param mParam;
	Settings* mSettingsPtr{nullptr};
};

bool AppSettingPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}

	if (mParam.mIsEnable == false) {
		return true;
	}

	if (mParam.mPrefix.IsEmpty()) {
		AfxMessageBox(_T("プレフィックスを入力してください"));
		return false;
	}

	// 正規表現として有効化をチェックする
	if (mParam.mExcludePattern.IsEmpty() == FALSE) {
		try {
			tregex regTmp((LPCTSTR)mParam.mExcludePattern);
		}
		catch(std::regex_error& e) {
			CString msg((LPCTSTR)IDS_ERR_INVALIDREGEXP);
			msg += _T("\n");

			CStringA what(e.what());
			msg += _T("\n");
			msg += (CString)what;
			msg += _T("\n");
			msg += mParam.mExcludePattern;
			AfxMessageBox(msg);
			return false;
		}
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
	DDX_Text(pDX, IDC_EDIT_PREFIX,  mParam.mPrefix);
	DDX_Text(pDX, IDC_EDIT_NUMOFRESULTS,  mParam.mNumOfResults);
	DDV_MinMaxInt(pDX,  mParam.mNumOfResults, 1, 128);
	DDX_Text(pDX, IDC_EDIT_SIZELIMIT,  mParam.mSizeLimit);
	DDV_MinMaxInt(pDX, mParam.mSizeLimit, 1, 128);
	DDX_Text(pDX, IDC_EDIT_COUNTLIMIT, mParam.mCountLimit);
	DDV_MinMaxInt(pDX, mParam.mCountLimit, 1, 131072);
	DDX_Text(pDX, IDC_EDIT_INTERVAL, mParam.mInterval);
	DDV_MinMaxInt(pDX, mParam.mInterval, 0, 5000);
	DDX_Text(pDX, IDC_EDIT_EXCLUDE, mParam.mExcludePattern);
	DDX_Check(pDX, IDC_CHECK_DISABLEMIGEMO, mParam.mIsDisableMigemo);
	DDX_Check(pDX, IDC_CHECK_PREVIEW, mParam.mUsePreview);
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
	BOOL isEnable = (mParam.mIsEnable == 1) ? TRUE : FALSE;
	GetDlgItem(IDC_EDIT_PREFIX)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_NUMOFRESULTS)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_SIZELIMIT)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_COUNTLIMIT)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_INTERVAL)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_EXCLUDE)->EnableWindow(isEnable);
	GetDlgItem(IDC_CHECK_DISABLEMIGEMO)->EnableWindow(isEnable);
	GetDlgItem(IDC_CHECK_PREVIEW)->EnableWindow(isEnable);
	return true;
}

void AppSettingPage::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	mParam.Load(*settingsPtr);
}

void AppSettingPage::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageClipboardHistory::PImpl
{
	AppSettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageClipboardHistory)

AppSettingPageClipboardHistory::AppSettingPageClipboardHistory() : 
	AppSettingPageBase(_T("拡張機能"), _T("クリップボード履歴")),
	in(new PImpl)
{
}

AppSettingPageClipboardHistory::~AppSettingPageClipboardHistory()
{
}

// ウインドウを作成する
bool AppSettingPageClipboardHistory::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_CLIPBOARD, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageClipboardHistory::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageClipboardHistory::GetOrder()
{
	return 100;
}
// 
bool AppSettingPageClipboardHistory::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageClipboardHistory::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageClipboardHistory::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageClipboardHistory::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageClipboardHistory::GetHelpPageId(String& id)
{
	id = "ClipboardHistorySetting";
	return true;
}


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

