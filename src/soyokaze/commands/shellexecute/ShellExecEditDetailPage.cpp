#include "pch.h"
#include "framework.h"
#include "ShellExecEditDetailPage.h"
#include "commands/shellexecute/ShellExecCommandParam.h"
#include "commands/common/Message.h"
#include "commands/core/CommandRepository.h"
#include "control/FolderDialog.h"
#include "control/DDXWrapper.h"
#include "icon/IconLabel.h"
#include "icon/IconLoader.h"
#include "utility/ShortcutFile.h"
#include "utility/Accessibility.h"
#include "utility/ProcessPath.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

ShellExecEditDetailPage::ShellExecEditDetailPage(CWnd* parentWnd) : 
	SettingPage(_T("その他"), IDD_SHELLEXECUTECOMMAND2, parentWnd)
{
}

ShellExecEditDetailPage::~ShellExecEditDetailPage()
{
}

void ShellExecEditDetailPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Check(pDX, IDC_CHECK_USE0, mParam.mIsUse0);
	DDX_Text(pDX, IDC_EDIT_PATH0, mParam.mNoParamAttr.mPath);
	DDX_Text(pDX, IDC_EDIT_PARAM0, mParam.mNoParamAttr.mParam);
	DDX_Control(pDX, IDC_BUTTON_MENU, mPathMenuBtn);
	DDX_Check(pDX, IDC_CHECK_ALLOWAUTOEXEC, mParam.mIsAllowAutoExecute);

	DDX_Check(pDX, IDC_CHECK_ACTIVATEWINDOW, mParam.mActivateWindowParam.mIsEnable);
	DDX_Text(pDX, IDC_EDIT_CAPTION, mParam.mActivateWindowParam.mCaptionStr);
	DDX_Text(pDX, IDC_EDIT_CLASS, mParam.mActivateWindowParam.mClassStr);
	DDX_Check(pDX, IDC_CHECK_REGEXP, mParam.mActivateWindowParam.mIsUseRegExp);
}

BEGIN_MESSAGE_MAP(ShellExecEditDetailPage, SettingPage)
	ON_EN_CHANGE(IDC_EDIT_PATH0, OnEditPath0Changed)
	ON_COMMAND(IDC_CHECK_USE0, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ACTIVATEWINDOW, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_RESOLVESHORTCUT2, OnButtonResolveShortcut0)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_MENU, OnPathMenuBtnClicked)
	ON_COMMAND(IDC_BUTTON_TEST, OnButtonTest)
	ON_EN_CHANGE(IDC_EDIT_CAPTION, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_CLASS, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_REGEXP, OnUpdateStatus)
	ON_MESSAGE(WM_APP+6, OnUserMessageCaptureWindow)

END_MESSAGE_MAP()


BOOL ShellExecEditDetailPage::OnInitDialog()
{
	__super::OnInitDialog();

	mMenuForPathBtn.CreatePopupMenu();
	mMenuForPathBtn.InsertMenu((UINT)-1, 0, 1, _T("ファイル選択"));
	mMenuForPathBtn.InsertMenu((UINT)-1, 0, 2, _T("フォルダ選択"));
	mPathMenuBtn.m_hMenu = (HMENU)mMenuForPathBtn;

	mIconLabel.SubclassDlgItem(IDC_STATIC_ICON, this);
	mIconLabel.DrawIcon(IconLoader::Get()->LoadWindowIcon());

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ShellExecEditDetailPage::UpdateStatus()
{
	// 引数あり/なしの制御
	GetDlgItem(IDC_STATIC_PATH0)->EnableWindow(mParam.mIsUse0);
	GetDlgItem(IDC_STATIC_PARAM00)->EnableWindow(mParam.mIsUse0);
	GetDlgItem(IDC_EDIT_PATH0)->EnableWindow(mParam.mIsUse0);
	GetDlgItem(IDC_EDIT_PARAM0)->EnableWindow(mParam.mIsUse0);
	GetDlgItem(IDC_BUTTON_MENU)->EnableWindow(mParam.mIsUse0);

	// ウインドウ切替の制御
	GetDlgItem(IDC_EDIT_CAPTION)->EnableWindow(mParam.mActivateWindowParam.mIsEnable);
	GetDlgItem(IDC_EDIT_CLASS)->EnableWindow(mParam.mActivateWindowParam.mIsEnable);
	GetDlgItem(IDC_CHECK_REGEXP)->EnableWindow(mParam.mActivateWindowParam.mIsEnable);
	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(mParam.mActivateWindowParam.mIsEnable);
	GetDlgItem(IDC_STATIC_ICON)->EnableWindow(mParam.mActivateWindowParam.mIsEnable);

	BOOL isShortcut0 = CString(_T(".lnk")).CompareNoCase(PathFindExtension(mParam.mNoParamAttr.mPath)) == 0;
	GetDlgItem(IDC_BUTTON_RESOLVESHORTCUT2)->ShowWindow(isShortcut0? SW_SHOW : SW_HIDE);


	if (mParam.mIsUse0 && mParam.mNoParamAttr.mPath.IsEmpty()) {
		mMessage.LoadString(IDS_ERR_PATH0ISEMPTY);
		DisalbleOKButton();
		return false;
	}

	mMessage.Empty();
	EnalbleOKButton();

	return true;
}

void ShellExecEditDetailPage::OnEditPath0Changed()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void ShellExecEditDetailPage::OnButtonBrowseFile2Clicked()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, mParam.mNoParamAttr.mPath, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mNoParamAttr.mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void ShellExecEditDetailPage::OnButtonBrowseDir2Clicked()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mNoParamAttr.mPath, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mNoParamAttr.mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void ShellExecEditDetailPage::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}


HBRUSH ShellExecEditDetailPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (::utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

BOOL ShellExecEditDetailPage::OnKillActive()
{
	// ToDo: UpdateStatusでfalseのケースではOKを許可しない
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL ShellExecEditDetailPage::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void ShellExecEditDetailPage::OnEnterSettings()
{
	auto param = (CommandParam*)GetParam();

	mParam = *param;
}

bool ShellExecEditDetailPage::GetHelpPageId(String& id)
{
	id = "ShellExecute_Detail";
	return true;
}



void ShellExecEditDetailPage::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	// Note : このページで設定するパラメータだけ書き戻す
	auto param = (CommandParam*)GetParam();

	param->mNoParamAttr.mPath = mParam.mNoParamAttr.mPath;
	param->mNoParamAttr.mParam = mParam.mNoParamAttr.mParam;
	param->mIsUse0 = mParam.mIsUse0;
	param->mIsAllowAutoExecute = mParam.mIsAllowAutoExecute;
	param->mActivateWindowParam = mParam.mActivateWindowParam;

	__super::OnOK();
}


void ShellExecEditDetailPage::ResolveShortcut(CString& path)
{
	UpdateData();
	auto resolvedPath = ShortcutFile::ResolvePath(path);
	if (resolvedPath.IsEmpty()) {
		CString msg((LPCTSTR)IDS_ERR_INVALIDSHORTCUT);
		msg += _T("\n");
		msg += path;
		AfxMessageBox(msg);
		return;
	}
	path = resolvedPath;
	UpdateStatus();
	UpdateData(FALSE);
}

void ShellExecEditDetailPage::OnButtonResolveShortcut0()
{
	ResolveShortcut(mParam.mNoParamAttr.mPath);
}

void ShellExecEditDetailPage::OnPathMenuBtnClicked()
{
	switch (mPathMenuBtn.m_nMenuResult) {
		case 1:
			OnButtonBrowseFile2Clicked();
			break;
		case 2:
			OnButtonBrowseDir2Clicked();
			break;
	}
}

void ShellExecEditDetailPage::OnButtonTest()
{
	UpdateData();

	if (UpdateStatus() == false) {
		return ;
	}

	if (mParam.mActivateWindowParam.mIsEnable == false) {
		return;
	}

	// 正規表現として入力された文字列が正しいかを確認
	CString errMsg;
	if (mParam.mActivateWindowParam.BuildCaptionRegExp(&errMsg) == false || 
	    mParam.mActivateWindowParam.BuildClassRegExp(&errMsg) == false) {
		PopupMessage(errMsg);
		return;
	}

	// ウインドウを探す
	HWND hwnd = mParam.mActivateWindowParam.FindHwnd();
	if (IsWindow(hwnd) == FALSE) {
		PopupMessage("ウインドウは見つかりませんでした");
		return;
	}

	FLASHWINFO fi;
	fi.cbSize = sizeof(fi);
	fi.hwnd = hwnd;
	fi.dwFlags = FLASHW_ALL;
	fi.uCount = 2;
	fi.dwTimeout = 500;
	::FlashWindowEx(&fi);
}

LRESULT
ShellExecEditDetailPage::OnUserMessageCaptureWindow(WPARAM pParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(pParam);

	HWND hTargetWnd = (HWND)lParam;
	if (IsWindow(hTargetWnd) == FALSE) {
		return 0;
	}
	ProcessPath processPath(hTargetWnd);

	// 自プロセスのウインドウなら何もしない
	if (GetCurrentProcessId() == processPath.GetProcessId()) {
		return 0;
	}

	HWND hwndRoot = ::GetAncestor(hTargetWnd, GA_ROOT);

	TCHAR caption[256];
	::GetWindowText(hwndRoot, caption, 256);
	TCHAR clsName[256];
	::GetClassName(hwndRoot, clsName, 256);

	mParam.mActivateWindowParam.mCaptionStr = caption;
	mParam.mActivateWindowParam.mClassStr = clsName;
	mParam.mActivateWindowParam.mIsUseRegExp = false;

	UpdateStatus();
	UpdateData(FALSE);

	try {
		CString path = processPath.GetProcessPath();
		mIconLabel.DrawIcon(IconLoader::Get()->GetDefaultIcon(path));
	}
	catch (ProcessPath::Exception&) {
		mIconLabel.DrawIcon(IconLoader::Get()->LoadWindowIcon());
	}
	return 0;
}

