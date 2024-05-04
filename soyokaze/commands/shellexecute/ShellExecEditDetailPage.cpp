#include "pch.h"
#include "framework.h"
#include "ShellExecEditDetailPage.h"
#include "commands/shellexecute/ShellExecCommandParam.h"
#include "gui/FolderDialog.h"
#include "icon/IconLabel.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "commands/core/CommandRepository.h"
#include "utility/ShortcutFile.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


ShellExecEditDetailPage::ShellExecEditDetailPage(CWnd* parentWnd) : 
	SettingPage(_T("詳細"), IDD_SHELLEXECUTECOMMAND2, parentWnd)
{
}

ShellExecEditDetailPage::~ShellExecEditDetailPage()
{
}

void ShellExecEditDetailPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_CBIndex(pDX, IDC_COMBO_SHOWTYPE, mParam.mShowType);
	DDX_Text(pDX, IDC_EDIT_PATH0, mParam.mPath0);
	DDX_Check(pDX, IDC_CHECK_USE0, mParam.mIsUse0);
	DDX_Text(pDX, IDC_EDIT_PARAM0, mParam.mParameter0);
	DDX_Text(pDX, IDC_EDIT_DIR, mParam.mDir);
}

BEGIN_MESSAGE_MAP(ShellExecEditDetailPage, SettingPage)
	ON_EN_CHANGE(IDC_EDIT_PATH0, OnEditPath0Changed)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE2, OnButtonBrowseFile2Clicked)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR2, OnButtonBrowseDir2Clicked)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR3, OnButtonBrowseDir3Clicked)
	ON_COMMAND(IDC_CHECK_USE0, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_RESOLVESHORTCUT2, OnButtonResolveShortcut0)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


BOOL ShellExecEditDetailPage::OnInitDialog()
{
	__super::OnInitDialog();

	// File&Folder Select Button
	GetDlgItem(IDC_BUTTON_BROWSEFILE2)->SetWindowTextW(L"\U0001F4C4");
	GetDlgItem(IDC_BUTTON_BROWSEDIR2)->SetWindowTextW(L"\U0001F4C2");
	GetDlgItem(IDC_BUTTON_BROWSEDIR3)->SetWindowTextW(L"\U0001F4C2");

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ShellExecEditDetailPage::UpdateStatus()
{
	GetDlgItem(IDC_STATIC_PATH0)->EnableWindow(mParam.mIsUse0);
	GetDlgItem(IDC_STATIC_PARAM00)->EnableWindow(mParam.mIsUse0);
	GetDlgItem(IDC_EDIT_PATH0)->EnableWindow(mParam.mIsUse0);
	GetDlgItem(IDC_EDIT_PARAM0)->EnableWindow(mParam.mIsUse0);
	GetDlgItem(IDC_BUTTON_BROWSEFILE2)->EnableWindow(mParam.mIsUse0);
	GetDlgItem(IDC_BUTTON_BROWSEDIR2)->EnableWindow(mParam.mIsUse0);

	BOOL isShortcut0 = CString(_T(".lnk")).CompareNoCase(PathFindExtension(mParam.mPath0)) == 0;
	GetDlgItem(IDC_BUTTON_RESOLVESHORTCUT2)->ShowWindow(isShortcut0? SW_SHOW : SW_HIDE);

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();

	if (mParam.mIsUse0 && mParam.mPath0.IsEmpty()) {
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
	CFileDialog dlg(TRUE, NULL, mParam.mPath0, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mPath0 = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void ShellExecEditDetailPage::OnButtonBrowseDir2Clicked()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mPath0, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mPath0 = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void ShellExecEditDetailPage::OnButtonBrowseDir3Clicked()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mDir, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mDir = dlg.GetPathName();
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
	if (utility::IsHighContrastMode()) {
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

bool ShellExecEditDetailPage::GetHelpPageId(CString& id)
{
	id = _T("ShellExecute_Detail");
	return true;
}



void ShellExecEditDetailPage::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	// ToDo: このページで設定するパラメータだけ書き戻す
	auto param = (CommandParam*)GetParam();
	param->mShowType = mParam.mShowType;
	param->mPath0 = mParam.mPath0;
	param->mIsUse0 = mParam.mIsUse0;
	param->mParameter0 = mParam.mParameter0;
	param->mDir = mParam.mDir;

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
	ResolveShortcut(mParam.mPath0);
}


