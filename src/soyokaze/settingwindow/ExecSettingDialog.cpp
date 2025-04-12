#include "pch.h"
#include "framework.h"
#include "ExecSettingDialog.h"
#include "setting/Settings.h"
#include "utility/DemotedProcessToken.h"  // for IsRunningAsAdmin()
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 
class ExecSettingDialog : public CDialog
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
	afx_msg void OnButtonBrowseFile();
	afx_msg void OnCheckUseFilter();

	// Ctrl+Enterキー実行でフォルダ表示する
	BOOL mIsShowFolderIfCtrlPressed = TRUE;
	// フォルダを開くファイラーを指定
	BOOL mIsUseExternalFiler = FALSE;
	// ファイル名を指定して実行を使用する
	BOOL mIsEnablePathFind = TRUE;
	// 管理者権限で着実行時にコマンド通常権限で実行する
	BOOL mShouldDemotePriviledge = TRUE;
	// 管理者権限で起動する
	BOOL mShouldRunAsAdmin = FALSE;
	// ファイラーのパス
	CString mFilerPath;
	// ファイラーのパラメータ
	CString mFilerParam;
	// 未登録キーワード実行時の動作
	int mDefaultActionIndex = 0;

	Settings* mSettingsPtr = nullptr;
};

bool ExecSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool ExecSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void ExecSettingDialog::OnOK()
{
	UpdateData();

	auto settingsPtr = mSettingsPtr;
	settingsPtr->Set(_T("Soyokaze:UseFiler"), (bool)mIsUseExternalFiler);
	settingsPtr->Set(_T("Soyokaze:FilerPath"), mFilerPath);
	settingsPtr->Set(_T("Soyokaze:FilerParam"), mFilerParam);
	settingsPtr->Set(_T("Soyokaze:ShouldDemotePriviledge"), (bool)mShouldDemotePriviledge);
	settingsPtr->Set(_T("Soyokaze:ShouldRunAsAdmin"), (bool)mShouldRunAsAdmin);
	settingsPtr->Set(_T("Soyokaze:IsShowFolderIfCtrlPressed"), (bool)mIsShowFolderIfCtrlPressed);
	settingsPtr->Set(_T("Soyokaze:IsEnablePathFind"), (bool)mIsEnablePathFind);

	CString defAction;
	if (mDefaultActionIndex == 1) {
		// FIXME: 今後機能を拡張するようなことがあれば別クラスに処理を移すこと
		defAction = _T("copy");
	}
	else if (mDefaultActionIndex == 2) {
		defAction = _T("register");
	}
	else {
		// 何もしない
		defAction = _T("");
	}

	settingsPtr->Set(_T("Launcher:DefaultActionType"), defAction); 

	__super::OnOK();
}

void ExecSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_USEFILER, mIsUseExternalFiler);
	DDX_Text(pDX, IDC_EDIT_FILERPATH, mFilerPath);
	DDX_Text(pDX, IDC_EDIT_FILERPARAM, mFilerParam);
	DDX_Check(pDX, IDC_CHECK_ENABLEPATHFIND, mIsEnablePathFind);
	DDX_Check(pDX, IDC_CHECK_ENABLEDEMOTEPRIVILEDGE, mShouldDemotePriviledge);
	DDX_Check(pDX, IDC_CHECK_RUNAS, mShouldRunAsAdmin);
	DDX_Check(pDX, IDC_CHECK_SHOWDIR, mIsShowFolderIfCtrlPressed);
	DDX_CBIndex(pDX, IDC_COMBO_DEFAULTACTION, mDefaultActionIndex);
}

BEGIN_MESSAGE_MAP(ExecSettingDialog, CDialog)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE, OnButtonBrowseFile)
	ON_COMMAND(IDC_CHECK_USEFILER, OnCheckUseFilter)
END_MESSAGE_MAP()

BOOL ExecSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// ファイル選択ボタン(絵文字にする)
	GetDlgItem(IDC_BUTTON_BROWSEFILE)->SetWindowTextW(L"\U0001F4C4");

	// 管理者特権のときに通常ユーザー権限で実行
	if (DemotedProcessToken::IsRunningAsAdmin()) {
		GetDlgItem(IDC_STATIC_RUNASONLY)->ShowWindow(SW_HIDE);
	}


	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ExecSettingDialog::UpdateStatus()
{
	// 外部ファイラーを使わない場合は関連する設定をグレーアウトする
	bool hasExternFilter = mIsUseExternalFiler;
	GetDlgItem(IDC_EDIT_FILERPATH)->EnableWindow(hasExternFilter);
	GetDlgItem(IDC_EDIT_FILERPARAM)->EnableWindow(hasExternFilter);
	GetDlgItem(IDC_BUTTON_BROWSEFILE)->EnableWindow(hasExternFilter);

	// 管理者特権のときに通常ユーザー権限で実行
	GetDlgItem(IDC_CHECK_ENABLEDEMOTEPRIVILEDGE)->EnableWindow(DemotedProcessToken::IsRunningAsAdmin());

	return true;
}

void ExecSettingDialog::OnButtonBrowseFile()
{
	UpdateData();

	CString filterStr((LPCTSTR)IDS_FILTER_EXE);
	CFileDialog dlg(TRUE, NULL, mFilerPath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mFilerPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void ExecSettingDialog::OnCheckUseFilter()
{
	UpdateData();
	UpdateStatus();
}

void ExecSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	mIsUseExternalFiler = settingsPtr->Get(_T("Soyokaze:UseFiler"), false);
	mFilerPath = settingsPtr->Get(_T("Soyokaze:FilerPath"), _T(""));
	mFilerParam = settingsPtr->Get(_T("Soyokaze:FilerParam"), _T(""));
	mShouldDemotePriviledge = settingsPtr->Get(_T("Soyokaze:ShouldDemotePriviledge"), true);
	mShouldRunAsAdmin = settingsPtr->Get(_T("Soyokaze:ShouldRunAsAdmin"), false);
	mIsShowFolderIfCtrlPressed = settingsPtr->Get(_T("Soyokaze:IsShowFolderIfCtrlPressed"), true);
	mIsEnablePathFind = settingsPtr->Get(_T("Soyokaze:IsEnablePathFind"), true);

	CString defAction = settingsPtr->Get(_T("Launcher:DefaultActionType"), _T("register")); 
	if (defAction == _T("copy")) {
		// FIXME: 今後機能を拡張するようなことがあれば別クラスに処理を移すこと
		mDefaultActionIndex = 1;
	}
	else if (defAction == _T("register")) {
		mDefaultActionIndex = 2;
	}
	else {
		// なにもしない
		mDefaultActionIndex = 0;
	}

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageExec::PImpl
{
	ExecSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageExec)

AppSettingPageExec::AppSettingPageExec() : 
	AppSettingPageBase(_T(""), _T("実行")),
	in(new PImpl)
{
}

AppSettingPageExec::~AppSettingPageExec()
{
}

// ウインドウを作成する
bool AppSettingPageExec::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_EXECSETTING, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageExec::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageExec::GetOrder()
{
	return 50;
}
// 
bool AppSettingPageExec::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageExec::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageExec::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageExec::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageExec::GetHelpPageId(CString& id)
{
	id = _T("ExecuteSetting");
	return true;
}

