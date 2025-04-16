#include "pch.h"
#include "framework.h"
#include "ShellExecEnvValueEditDialog.h"
#include "app/Manual.h"
#include "gui/FolderDialog.h"
#include "utility/Accessibility.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace shellexecute {

ValueEditDialog::ValueEditDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_SHELLEXEC_ENVEDIT, parentWnd)
{
	SetHelpPageId(_T("ShellExecEnvEdit"));
}

ValueEditDialog::~ValueEditDialog()
{
}

CString ValueEditDialog::GetName()
{
	return mName;
}

void ValueEditDialog::SetName(const CString& name)
{
	mName = name;
	// 新規作成でない場合は名前の編集を許可しない
	mCanEditName = false;
}

CString ValueEditDialog::GetValue()
{
	return mValue;
}

void ValueEditDialog::SetValue(const CString& value)
{
	mValue = value;
}

bool ValueEditDialog::ValidateNameAndValue(const CString& name, const CString& value)
{
	// 変数名に使用できない文字はスペースと等号記号
	if (name.FindOneOf(_T(" =")) != -1) {
		mMessage = _T("環境変数名に使用できない文字が含まれています。");
		return false;
	}
	if (name.IsEmpty()) {
		mMessage = _T("環境変数名を入力してください。");
		return false;
	}

	// 変数名の長さ
	if (name.GetLength() >= 256) {
		mMessage = _T("環境変数名が長すぎます。\n(このアプリでの上限は255文字です)");
		return false;
	}

	// 値の長さ
	if (value.GetLength() >= 4096) {
		mMessage = _T("値が長すぎます。\n(このアプリでの上限は4095文字です)");
		return false;
	}

	mMessage.Empty();
	return true;
}

void ValueEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mName);
	DDX_Text(pDX, IDC_EDIT_VALUE, mValue);

	if (pDX->m_bSaveAndValidate == FALSE) {
		return;
	}

	// 名前と値の検証
	bool isValidateOK = ValidateNameAndValue(mName, mValue);
	GetDlgItem(IDOK)->EnableWindow(isValidateOK);
}

#pragma warning( push )

BEGIN_MESSAGE_MAP(ValueEditDialog, launcherapp::gui::SinglePageDialog)
	ON_WM_CTLCOLOR()
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_VALUE, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE, OnButtonBrowseFile)
	ON_COMMAND(IDC_BUTTON_BROWSEDIR, OnButtonBrowseDir)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL ValueEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	GetDlgItem(IDC_EDIT_NAME)->EnableWindow(mCanEditName);

	UpdateData();
	UpdateData(FALSE);

	return TRUE;
}

void ValueEditDialog::OnOK()
{
	if (UpdateData() == FALSE) {
		return;
	}
	__super::OnOK();
}

HBRUSH ValueEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void ValueEditDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateData(FALSE);
}

void ValueEditDialog::OnButtonBrowseFile()
{
	UpdateData();

	CString initialDir;
	if (Path::FileExists(mValue)) {
		initialDir = mValue;
	}
	CFileDialog dlg(TRUE, nullptr, initialDir, OFN_NOCHANGEDIR);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mValue = dlg.GetPathName();
	UpdateData(FALSE);
}

void ValueEditDialog::OnButtonBrowseDir()
{
	UpdateData();

	CString initialDir;
	if (Path::IsDirectory(mValue)) {
		initialDir = mValue;
	}
	else if (Path::FileExists(mValue)) {
		Path tmp(mValue);
		tmp.RemoveFileSpec();
		initialDir = tmp;
	}
	CFolderDialog dlg(_T(""), initialDir, this);

	if (dlg.DoModal() != IDOK) {
		return;
	}

	mValue = dlg.GetPathName();
	UpdateData(FALSE);
}

// マニュアル表示
void ValueEditDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("MacroList"));
	*pResult = 0;
}


}}} // end of namespace launcherapp::commands::shellexecute
