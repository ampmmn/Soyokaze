#include "pch.h"
#include "ArgumentDialog.h"
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace shellexecute {



ArgumentDialog::ArgumentDialog(
	const CString& cmdName,
	CWnd* parentWnd
) :  CDialogEx(IDD_ARGUMENTS, parentWnd), mCommandName(cmdName)
{
}

ArgumentDialog::~ArgumentDialog()
{
}

const CString& ArgumentDialog::GetArguments()
{
	return mArguments;
}

void ArgumentDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ARGUMENTS, mArguments);
}

BEGIN_MESSAGE_MAP(ArgumentDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_ARGUMENTS, OnUpdateStatus)
END_MESSAGE_MAP()

BOOL ArgumentDialog::OnInitDialog()
{
	__super::OnInitDialog();

	CString caption;
	GetWindowText(caption);
	if (mCommandName.IsEmpty() == FALSE) {
		CString tmp;
		tmp.Format(_T("【%s】"), (LPCTSTR)mCommandName);
		caption += tmp;
		SetWindowText(caption);
	}

	OnUpdateStatus();

	return TRUE;
}

void ArgumentDialog::OnOK()
{
	UpdateData();
	__super::OnOK();
}

void ArgumentDialog::OnUpdateStatus()
{
	UpdateData();
	GetDlgItem(IDOK)->EnableWindow(mArguments.IsEmpty() == FALSE);
}

}
}
}

