#include "pch.h"
#include "framework.h"
#include "SubProcessDialog.h"
#include "gui/FolderDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/ExpandFunctions.h"
#include "utility/Accessibility.h"
#include "utility/Path.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace common {

SubProcessDialog::SubProcessDialog(LPCTSTR helpId, CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_FILTER_AFTER_SUBPROCESS, parentWnd)
{
	SetHelpPageId(helpId);
}

SubProcessDialog::~SubProcessDialog()
{
}

void SubProcessDialog::SetParam(const Param& param)
{
	mParam = param;
}

const SubProcessDialog::Param& SubProcessDialog::GetParam()
{
	return mParam;
}

void SubProcessDialog::SetVariableDescription(LPCTSTR text)
{
	mVariableText = text;
}


void SubProcessDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_PATH2, mParam.mFilePath);
	DDX_Text(pDX, IDC_EDIT_PARAM2, mParam.mCommandParam);
	DDX_Control(pDX, IDC_BUTTON_MENU, mPathMenuBtn);
	DDX_Text(pDX, IDC_EDIT_DIR, mParam.mWorkDir);
	DDX_CBIndex(pDX, IDC_COMBO_SHOWTYPE, mParam.mShowType);
	DDX_Text(pDX, IDC_STATIC_VARIABLE, mVariableText);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(SubProcessDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR, OnSelectWorkDir)
	ON_EN_CHANGE(IDC_EDIT_PATH2, OnUpdateStatus)
	ON_BN_CLICKED(IDC_BUTTON_MENU, OnPathMenuBtnClicked)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL SubProcessDialog::OnInitDialog()
{
	__super::OnInitDialog();

	GetDlgItem(IDC_BUTTON_BROWSEDIR)->SetWindowTextW(L"\U0001F4C2");

	mMenuForPathBtn.CreatePopupMenu();
	mMenuForPathBtn.InsertMenu((UINT)-1, 0, 1, _T("ファイル選択"));
	mMenuForPathBtn.InsertMenu((UINT)-1, 0, 2, _T("フォルダ選択"));
	mPathMenuBtn.m_hMenu = (HMENU)mMenuForPathBtn;

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool SubProcessDialog::UpdateStatus()
{
	if (mParam.mFilePath.IsEmpty()) {
		mMessage = _T("実行するファイルまたはURLを入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	CString workDirStr = mParam.mWorkDir;
	ExpandMacros(workDirStr);

	Path workDir(workDirStr);
	if (workDirStr.IsEmpty() == FALSE && workDir.IsDirectory() == false) {
		mMessage = _T("作業フォルダは存在しません");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	// パスが有効なファイルだったら編集メニューを表示する
	mMenuForPathBtn.DeleteMenu(3, MF_BYCOMMAND);
	if (IsEditableFileType(mParam.mFilePath)) {
			mMenuForPathBtn.InsertMenu((UINT)-1, 0, 3, _T("編集"));
	}

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	return true;
}

void SubProcessDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH SubProcessDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void SubProcessDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	//mParam.mPostFilterType = 1;

	__super::OnOK();
}

void SubProcessDialog::OnSelectFilePath()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, mParam.mFilePath, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mFilePath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void SubProcessDialog::OnSelectFolderPath()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mFilePath, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mFilePath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void SubProcessDialog::OnPathMenuBtnClicked()
{
	switch (mPathMenuBtn.m_nMenuResult) {
		case 1:
			OnSelectFilePath();
			break;
		case 2:
			OnSelectFolderPath();
			break;
		case 3:
			OpenTarget();
			break;
	}
}

// (テキストエディタなどで)編集するようなファイルタイプか?
bool SubProcessDialog::IsEditableFileType(CString pathStr)
{
	ExpandMacros(pathStr);

	Path path(pathStr);
	if (path.FileExists() == false || path.IsDirectory()) {
		// ファイルが存在しない、あるいは、パスはディレクトリであるため、編集できない
		return false;
	}

	CString ext(PathFindExtension(path));

	// 実行ファイルはバイナリ形式なので普通は編集しない
	if (ext.CompareNoCase(_T(".exe")) == 0) {
		return false;
	}

	// それ以外は編集する可能性がある
	return true;
}

void SubProcessDialog::OpenTarget()
{
	CString path(mParam.mFilePath);
	ExpandMacros(path);

	auto p = path.GetBuffer(path.GetLength() + 1);

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = p;

	TCHAR verb[24];
	_tcscpy_s(verb, _T("edit"));
	si.lpVerb = verb;

	ShellExecuteEx(&si);

	path.ReleaseBuffer();

	if (si.hProcess != nullptr) {
		CloseHandle(si.hProcess);
	}
}

void SubProcessDialog::OnSelectWorkDir()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mWorkDir, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mWorkDir = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

