#include "pch.h"
#include "framework.h"
#include "ShellExecEditDetailPage.h"
#include "commands/shellexecute/ShellExecCommandParam.h"
#include "gui/FolderDialog.h"
#include "icon/IconLabel.h"
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
}

BEGIN_MESSAGE_MAP(ShellExecEditDetailPage, SettingPage)
	ON_EN_CHANGE(IDC_EDIT_PATH0, OnEditPath0Changed)
	ON_COMMAND(IDC_CHECK_USE0, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_RESOLVESHORTCUT2, OnButtonResolveShortcut0)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_MENU, OnPathMenuBtnClicked)
END_MESSAGE_MAP()


BOOL ShellExecEditDetailPage::OnInitDialog()
{
	__super::OnInitDialog();

	mMenuForPathBtn.CreatePopupMenu();
	mMenuForPathBtn.InsertMenu((UINT)-1, 0, 1, _T("ファイル選択"));
	mMenuForPathBtn.InsertMenu((UINT)-1, 0, 2, _T("フォルダ選択"));
	mPathMenuBtn.m_hMenu = (HMENU)mMenuForPathBtn;


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
	GetDlgItem(IDC_BUTTON_MENU)->EnableWindow(mParam.mIsUse0);

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

	// Note : このページで設定するパラメータだけ書き戻す
	auto param = (CommandParam*)GetParam();

	param->mNoParamAttr.mPath = mParam.mNoParamAttr.mPath;
	param->mNoParamAttr.mParam = mParam.mNoParamAttr.mParam;
	param->mIsUse0 = mParam.mIsUse0;
	param->mIsAllowAutoExecute = mParam.mIsAllowAutoExecute;

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


