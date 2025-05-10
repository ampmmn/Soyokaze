#include "pch.h"
#include "framework.h"
#include "PreFilterSubProcessDialog.h"
#include "gui/FolderDialog.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/ExecutablePath.h"
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
namespace filter {

PreFilterSubProcessDialog::PreFilterSubProcessDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_FILTER_SUBPROCESS, parentWnd),
	mPreFilterCodePageIndex(0)
{
	SetHelpPageId(_T("PreFilterSubprocess"));
}

PreFilterSubProcessDialog::~PreFilterSubProcessDialog()
{
}

void PreFilterSubProcessDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

const CommandParam& PreFilterSubProcessDialog::GetParam()
{
	return mParam;
}

void PreFilterSubProcessDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_PATH, mParam.mPath);
	DDX_Text(pDX, IDC_EDIT_PARAM, mParam.mParameter);
	DDX_Text(pDX, IDC_EDIT_DIR, mParam.mDir);
	DDX_CBIndex(pDX, IDC_COMBO_PREFILTERCODEPAGE, mPreFilterCodePageIndex);
	DDX_Check(pDX, IDC_CHECK_CACHE, mParam.mCacheType);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(PreFilterSubProcessDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_PATH, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_DIR, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE1, OnButtonBrowseFile1Clicked)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR3, OnButtonBrowseDir3Clicked)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL PreFilterSubProcessDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// File&Folder Select Button
	GetDlgItem(IDC_BUTTON_BROWSEFILE1)->SetWindowTextW(L"\U0001F4C4");
	GetDlgItem(IDC_BUTTON_BROWSEDIR3)->SetWindowTextW(L"\U0001F4C2");

	// 前段フィルタの文字コード
	if (mParam.mPreFilterCodePage == CP_UTF8) {
		mPreFilterCodePageIndex = 0;
	}
	else if (mParam.mPreFilterCodePage == 932) {   // codepage 932 → sjis
		mPreFilterCodePageIndex = 1;
	}
	else {
		// その他の値はUTF-8扱い
		mPreFilterCodePageIndex = 0;
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool PreFilterSubProcessDialog::UpdateStatus()
{
	if (mParam.mPath.IsEmpty()) {
		mMessage.LoadString(IDS_ERR_PATHISEMPTY);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	ExecutablePath path(mParam.mPath);
	if (path.IsExecutable() == false) {
		mMessage = _T("ファイルが存在しません。");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	CString workDirStr = mParam.mDir;
	ExpandMacros(workDirStr);

	Path workDir(workDirStr);
	if (workDirStr.IsEmpty() == FALSE && workDir.IsDirectory() == false) {
		mMessage = _T("作業フォルダは存在しません");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	return true;
}

void PreFilterSubProcessDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void PreFilterSubProcessDialog::OnButtonBrowseFile1Clicked()
{
	UpdateData();
	CFileDialog dlg(TRUE, NULL, mParam.mPath, OFN_FILEMUSTEXIST, _T("All files|*.*||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mParam.mPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void PreFilterSubProcessDialog::OnButtonBrowseDir3Clicked()
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

HBRUSH PreFilterSubProcessDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void PreFilterSubProcessDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	if (mPreFilterCodePageIndex == 0) {
		mParam.mPreFilterCodePage = CP_UTF8;
	}
	else if (mPreFilterCodePageIndex == 1) {
		mParam.mPreFilterCodePage = 932;
	}
	else {
		// その他の値はUTF-8扱い
		mParam.mPreFilterCodePage = CP_UTF8;
	}

	mParam.mPreFilterType = 0;

	__super::OnOK();
}

// マニュアル表示
void PreFilterSubProcessDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("MacroList"));
	*pResult = 0;
}

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

