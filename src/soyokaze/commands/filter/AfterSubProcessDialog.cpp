#include "pch.h"
#include "framework.h"
#include "AfterSubProcessDialog.h"
#include "gui/FolderDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/ExpandFunctions.h"
#include "utility/Accessibility.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace filter {

AfterSubProcessDialog::AfterSubProcessDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_FILTER_AFTER_SUBPROCESS, parentWnd)
{
	SetHelpPageId(_T("PostFilterSubProcess"));
}

AfterSubProcessDialog::~AfterSubProcessDialog()
{
}

void AfterSubProcessDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

const CommandParam& AfterSubProcessDialog::GetParam()
{
	return mParam;
}

void AfterSubProcessDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_PATH2, mParam.mAfterFilePath);
	DDX_Text(pDX, IDC_EDIT_PARAM2, mParam.mAfterCommandParam);
	DDX_Control(pDX, IDC_BUTTON_MENU, mPathMenuBtn);
	DDX_Text(pDX, IDC_EDIT_DIR, mParam.mAfterDir);
	DDX_CBIndex(pDX, IDC_COMBO_SHOWTYPE, mParam.mAfterShowType);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AfterSubProcessDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR, OnSelectWorkDir)
	ON_EN_CHANGE(IDC_EDIT_PATH2, OnUpdateStatus)
	ON_BN_CLICKED(IDC_BUTTON_MENU, OnPathMenuBtnClicked)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL AfterSubProcessDialog::OnInitDialog()
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

bool AfterSubProcessDialog::UpdateStatus()
{
	if (mParam.mAfterFilePath.IsEmpty()) {
		mMessage = _T("絞込み後に実行するファイルまたはURLを入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	CString workDir = mParam.mAfterDir;
	ExpandMacros(workDir);

	if (workDir.IsEmpty() == FALSE && PathIsDirectory(workDir) == FALSE) {
		mMessage = _T("作業フォルダは存在しません");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	// パスが有効なファイルだったら編集メニューを表示する
	mMenuForPathBtn.DeleteMenu(3, MF_BYCOMMAND);
	if (IsEditableFileType(mParam.mAfterFilePath)) {
			mMenuForPathBtn.InsertMenu((UINT)-1, 0, 3, _T("編集"));
	}

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	return true;
}

void AfterSubProcessDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH AfterSubProcessDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void AfterSubProcessDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	mParam.mPostFilterType = 1;

	__super::OnOK();
}

void AfterSubProcessDialog::OnSelectFilePath()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, mParam.mAfterFilePath, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mAfterFilePath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void AfterSubProcessDialog::OnSelectFolderPath()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mAfterFilePath, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mAfterFilePath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void AfterSubProcessDialog::OnPathMenuBtnClicked()
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
bool AfterSubProcessDialog::IsEditableFileType(CString path)
{
	ExpandMacros(path);
	if (PathFileExists(path) == FALSE || PathIsDirectory(path)) {
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

void AfterSubProcessDialog::OpenTarget()
{
	CString path(mParam.mAfterFilePath);
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

void AfterSubProcessDialog::OnSelectWorkDir()
{
	UpdateData();
	CFolderDialog dlg(_T(""), mParam.mAfterDir, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mAfterDir = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}


} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

