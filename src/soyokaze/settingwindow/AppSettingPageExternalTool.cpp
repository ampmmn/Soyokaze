#include "pch.h"
#include "framework.h"
#include "AppSettingPageExternalTool.h"
#include "setting/Settings.h"
#include "control/FolderDialog.h"
#include "control/DDXWrapper.h"
#include "externaltool/webbrowser/ChromeEnvironment.h"
#include "utility/Path.h"
#include "utility/Accessibility.h"
#include "utility/VersionInfo.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ChromeEnvironment = launcherapp::externaltool::webbrowser::ChromeEnvironment;

// 
class ExternalToolSettingDialog : public CDialog
{
public:
	bool UpdateStatus();

	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

	static bool IsValidDataFolder(const CString& folderPath);

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnCheckWarnLongTime();
	afx_msg void OnButtonExeFileBrowse();
	afx_msg void OnButtonDataDirBrowse();
	afx_msg void OnUpdateStatus();

	// 外部ブラウザの登録を有効にする
	bool mIsEnable{false};
	// URLを開くときに外部ブラウザを使うか
	bool mShouldUseExternal{false};

	// 0:Chrome 1:Chrome以外の外部ブラウザ
	int mExternalBrowserType{0};

	// 実行ファイルのパス
	CString mExeFilePath;
	// 実行時パラメータ
	CString mParameter;
	// 設定フォルダのパス
	CString mUserDataDirPath;
	// アプリの表示名
	CString mDisplayName;

	// メッセージ
	CString mStatusMsg;

	Settings* mSettingsPtr{nullptr};
};

bool ExternalToolSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}

	return true;
}

bool ExternalToolSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void ExternalToolSettingDialog::OnOK()
{
	if (UpdateData() == FALSE) {
		return;
	}

	bool useChrome = mExternalBrowserType == 0;
	auto settingsPtr = mSettingsPtr;
	settingsPtr->Set(_T("ExternalToolBrowser:EnableExternal"), mIsEnable);
	settingsPtr->Set(_T("ExternalToolBrowser:UseChrome"), useChrome);
	settingsPtr->Set(_T("ExternalToolBrowser:ExeFilePath"), mExeFilePath);
	settingsPtr->Set(_T("ExternalToolBrowser:Parameter"), mParameter);
	settingsPtr->Set(_T("ExternalToolBrowser:DataDirPath"), mUserDataDirPath);
	settingsPtr->Set(_T("ExternalToolBrowser:DisplayName"), mDisplayName);
	// ダイアログの文言は「コマンドからURLを開くとき、既定のブラウザではなく、上記のWebブラウザを使う」としているが
	// 内部的には「既定のブラウザを使う」という扱いにしたいので条件を反転する
	settingsPtr->Set(_T("ExternalToolBrowser:OpenUrlWithSystem"), !mShouldUseExternal);

	__super::OnOK();
}

void ExternalToolSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_USEEXTERNAL, mIsEnable);
	DDX_Radio(pDX, IDC_RADIO_USECHROME, mExternalBrowserType);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, mExeFilePath);
	DDX_Text(pDX, IDC_EDIT_PARAMETER, mParameter);
	DDX_Text(pDX, IDC_EDIT_DATADIR, mUserDataDirPath);
	DDX_Text(pDX, IDC_EDIT_DISPLAYNAME, mDisplayName);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mStatusMsg);
	DDX_Check(pDX, IDC_CHECK_OPENURLINOTHERBROWSER, mShouldUseExternal);
}

BEGIN_MESSAGE_MAP(ExternalToolSettingDialog, CDialog)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_CHECK_USEEXTERNAL, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSE, OnButtonExeFileBrowse)
	ON_COMMAND(IDC_BUTTON_BROWSE2, OnButtonDataDirBrowse)
	ON_COMMAND(IDC_RADIO_USECHROME, OnUpdateStatus)
	ON_COMMAND(IDC_RADIO_USEOTHER, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_FILEPATH, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_DATADIR, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_DISPLAYNAME, OnUpdateStatus)
END_MESSAGE_MAP()


BOOL ExternalToolSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

/**
 *  エラーの時に一部コントロールの色を変える
 */
HBRUSH ExternalToolSettingDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (::utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mStatusMsg.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}

	return br;
}

bool ExternalToolSettingDialog::UpdateStatus()
{
	mStatusMsg.Empty();

	bool isEnable = mIsEnable;
	bool useNonChrome = mExternalBrowserType != 0;

	// チェック状態に応じてコントロールをグレーアウト
	GetDlgItem(IDC_RADIO_USECHROME)->EnableWindow(isEnable);
	GetDlgItem(IDC_RADIO_USEOTHER)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_FILEPATH)->EnableWindow(isEnable && useNonChrome);
	GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(isEnable && useNonChrome);
	GetDlgItem(IDC_EDIT_PARAMETER)->EnableWindow(isEnable && useNonChrome);
	GetDlgItem(IDC_EDIT_DATADIR)->EnableWindow(isEnable && useNonChrome);
	GetDlgItem(IDC_EDIT_DISPLAYNAME)->EnableWindow(isEnable && useNonChrome);
	GetDlgItem(IDC_BUTTON_BROWSE2)->EnableWindow(isEnable && useNonChrome);
	GetDlgItem(IDC_CHECK_OPENURLINOTHERBROWSER)->EnableWindow(isEnable);

	// この画面で設定する機能を利用する場合に環境が要件を満たしているか確認する
	if (isEnable) {
		// mExternalBrowserTypeが0だったら、Chromeが利用可能か確認する。
		// 利用可能でなければステータス欄にChromeがインストールされていない旨を表示
		if (mExternalBrowserType == 0) {
			auto chrome = ChromeEnvironment::GetInstance();
			mStatusMsg = chrome->IsAvailable() ? _T("") : _T("Chromeがインストールされていません");
			return false;
		}
		else {
			// 実行ファイルのパスが設定されていなかったらその旨を表示する
			if (mExeFilePath.IsEmpty()) {
				mStatusMsg = _T("実行ファイルのパスを指定してください");
			}
			else if (Path::FileExists(mExeFilePath) == false) {
				mStatusMsg = _T("指定された実行ファイルは存在しません");
			}
			else if (mUserDataDirPath.IsEmpty()) {
				mStatusMsg = _T("設定データフォルダのパスを指定してください");
			}
			else if (Path::IsDirectory(mUserDataDirPath) == false) {
				mStatusMsg = _T("指定された設定データフォルダは存在しません");
			}
			else if (mDisplayName.IsEmpty()) {
				mStatusMsg = _T("アプリの表示名を入力してください");
			}
			else if (IsValidDataFolder(mUserDataDirPath) == false) {
				mStatusMsg = _T("有効な設定データフォルダではありません");
			}
		}
		return false;
	}

	return true;
}


void ExternalToolSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

	mIsEnable = settingsPtr->Get(_T("ExternalToolBrowser:EnableExternal"), true);

	bool useChrome = settingsPtr->Get(_T("ExternalToolBrowser:UseChrome"), true);
	mExternalBrowserType = useChrome ? 0 : 1;
	
	mExeFilePath = settingsPtr->Get(_T("ExternalToolBrowser:ExeFilePath"), _T(""));
	mParameter = settingsPtr->Get(_T("ExternalToolBrowser:Parameter"), _T("$target"));
	mUserDataDirPath = settingsPtr->Get(_T("ExternalToolBrowser:DataDirPath"), _T(""));
	mDisplayName = settingsPtr->Get(_T("ExternalToolBrowser:DisplayName"), _T(""));
	mShouldUseExternal = settingsPtr->Get(_T("ExternalToolBrowser:OpenUrlWithSystem"), true) == false;
}


void ExternalToolSettingDialog::OnCheckWarnLongTime()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void ExternalToolSettingDialog::OnButtonExeFileBrowse()
{
	UpdateData();

	CFileDialog dlg(TRUE, nullptr, mExeFilePath, OFN_FILEMUSTEXIST, _T("実行ファイル (*.exe)|*.exe||"), this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	mExeFilePath = dlg.GetPathName();

	CString displayName;
	if (VersionInfo::GetProductName(mExeFilePath, displayName)) {
		mDisplayName = displayName;
	}

	UpdateStatus();
	UpdateData(FALSE);
}

void ExternalToolSettingDialog::OnButtonDataDirBrowse()
{
	UpdateData();

	CFolderDialog dlg(_T(""), mUserDataDirPath, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mUserDataDirPath = dlg.GetPathName();

	UpdateStatus();
	UpdateData(FALSE);
}

void ExternalToolSettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

bool ExternalToolSettingDialog::IsValidDataFolder(const CString& folderPath)
{
	if (Path::IsDirectory(folderPath) == false) {
		return false;
	}

	Path path(folderPath);
	path.Append(_T("Default\\Bookmarks"));
	if (path.FileExists() == false) {
		return false;
	}

	path = folderPath;
	path.Append(_T("Default\\History"));
	if (path.FileExists() == false) {
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageExternalTool::PImpl
{
	ExternalToolSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageExternalTool)

AppSettingPageExternalTool::AppSettingPageExternalTool() : 
	AppSettingPageBase(_T(""), _T("外部ツール")),
	in(new PImpl)
{
}

AppSettingPageExternalTool::~AppSettingPageExternalTool()
{
}

// ウインドウを作成する
bool AppSettingPageExternalTool::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_EXTERNALTOOL, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageExternalTool::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageExternalTool::GetOrder()
{
	return 80;
}
// 
bool AppSettingPageExternalTool::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageExternalTool::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageExternalTool::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageExternalTool::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageExternalTool::GetHelpPageId(String& id)
{
	id = "ExternalToolSetting";
	return true;
}

