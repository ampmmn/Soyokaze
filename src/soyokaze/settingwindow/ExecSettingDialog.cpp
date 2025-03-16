#include "pch.h"
#include "framework.h"
#include "ExecSettingDialog.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct ExecSettingDialog::PImpl
{
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
};

ExecSettingDialog::ExecSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("実行"), IDD_EXECSETTING, parentWnd), in(new PImpl)
{
}

ExecSettingDialog::~ExecSettingDialog()
{
}

BOOL ExecSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL ExecSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void ExecSettingDialog::OnOK()
{
	UpdateData();

	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("Soyokaze:UseFiler"), (bool)in->mIsUseExternalFiler);
	settingsPtr->Set(_T("Soyokaze:FilerPath"), in->mFilerPath);
	settingsPtr->Set(_T("Soyokaze:FilerParam"), in->mFilerParam);
	settingsPtr->Set(_T("Soyokaze:ShouldDemotePriviledge"), (bool)in->mShouldDemotePriviledge);
	settingsPtr->Set(_T("Soyokaze:ShouldRunAsAdmin"), (bool)in->mShouldRunAsAdmin);
	settingsPtr->Set(_T("Soyokaze:IsShowFolderIfCtrlPressed"), (bool)in->mIsShowFolderIfCtrlPressed);
	settingsPtr->Set(_T("Soyokaze:IsEnablePathFind"), (bool)in->mIsEnablePathFind);

	CString defAction;
	if (in->mDefaultActionIndex == 1) {
		// FIXME: 今後機能を拡張するようなことがあれば別クラスに処理を移すこと
		defAction = _T("copy");
	}
	else if (in->mDefaultActionIndex == 2) {
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

	DDX_Check(pDX, IDC_CHECK_USEFILER, in->mIsUseExternalFiler);
	DDX_Text(pDX, IDC_EDIT_FILERPATH, in->mFilerPath);
	DDX_Text(pDX, IDC_EDIT_FILERPARAM, in->mFilerParam);
	DDX_Check(pDX, IDC_CHECK_ENABLEPATHFIND, in->mIsEnablePathFind);
	DDX_Check(pDX, IDC_CHECK_ENABLEDEMOTEPRIVILEDGE, in->mShouldDemotePriviledge);
	DDX_Check(pDX, IDC_CHECK_RUNAS, in->mShouldRunAsAdmin);
	DDX_Check(pDX, IDC_CHECK_SHOWDIR, in->mIsShowFolderIfCtrlPressed);
	DDX_CBIndex(pDX, IDC_COMBO_DEFAULTACTION, in->mDefaultActionIndex);
}

BEGIN_MESSAGE_MAP(ExecSettingDialog, SettingPage)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE, OnButtonBrowseFile)
	ON_COMMAND(IDC_CHECK_USEFILER, OnCheckUseFilter)
END_MESSAGE_MAP()

// 管理者権限で実行されているか?
static bool IsRunningAsAdmin()
{
	static bool isRunAsAdmin = []() {
		PSID grp;
		SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
		BOOL result = AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &grp);
		if (result == FALSE) {
			return false;
		}

		BOOL isMember = FALSE;
		result = CheckTokenMembership(nullptr, grp, &isMember);
		FreeSid(grp);

		return result && isMember;
	}();
	return isRunAsAdmin;
}

BOOL ExecSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// ファイル選択ボタン(絵文字にする)
	GetDlgItem(IDC_BUTTON_BROWSEFILE)->SetWindowTextW(L"\U0001F4C4");

	// 管理者特権のときに通常ユーザー権限で実行
	if (IsRunningAsAdmin()) {
		GetDlgItem(IDC_STATIC_RUNASONLY)->ShowWindow(SW_HIDE);
	}


	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ExecSettingDialog::UpdateStatus()
{
	// 外部ファイラーを使わない場合は関連する設定をグレーアウトする
	bool hasExternFilter = in->mIsUseExternalFiler;
	GetDlgItem(IDC_EDIT_FILERPATH)->EnableWindow(hasExternFilter);
	GetDlgItem(IDC_EDIT_FILERPARAM)->EnableWindow(hasExternFilter);
	GetDlgItem(IDC_BUTTON_BROWSEFILE)->EnableWindow(hasExternFilter);

	// 管理者特権のときに通常ユーザー権限で実行
	GetDlgItem(IDC_CHECK_ENABLEDEMOTEPRIVILEDGE)->EnableWindow(IsRunningAsAdmin());

	return true;
}

void ExecSettingDialog::OnButtonBrowseFile()
{
	UpdateData();

	CString filterStr((LPCTSTR)IDS_FILTER_EXE);
	CFileDialog dlg(TRUE, NULL, in->mFilerPath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	in->mFilerPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void ExecSettingDialog::OnCheckUseFilter()
{
	UpdateData();
	UpdateStatus();
}

void ExecSettingDialog::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();
	in->mIsUseExternalFiler = settingsPtr->Get(_T("Soyokaze:UseFiler"), false);
	in->mFilerPath = settingsPtr->Get(_T("Soyokaze:FilerPath"), _T(""));
	in->mFilerParam = settingsPtr->Get(_T("Soyokaze:FilerParam"), _T(""));
	in->mShouldDemotePriviledge = settingsPtr->Get(_T("Soyokaze:ShouldDemotePriviledge"), true);
	in->mShouldRunAsAdmin = settingsPtr->Get(_T("Soyokaze:ShouldRunAsAdmin"), false);
	in->mIsShowFolderIfCtrlPressed = settingsPtr->Get(_T("Soyokaze:IsShowFolderIfCtrlPressed"), true);
	in->mIsEnablePathFind = settingsPtr->Get(_T("Soyokaze:IsEnablePathFind"), true);

	CString defAction = settingsPtr->Get(_T("Launcher:DefaultActionType"), _T("register")); 
	if (defAction == _T("copy")) {
		// FIXME: 今後機能を拡張するようなことがあれば別クラスに処理を移すこと
		in->mDefaultActionIndex = 1;
	}
	else if (defAction == _T("register")) {
		in->mDefaultActionIndex = 2;
	}
	else {
		// なにもしない
		in->mDefaultActionIndex = 0;
	}

}

bool ExecSettingDialog::GetHelpPageId(CString& id)
{
	id = _T("ExecuteSetting");
	return true;
}

