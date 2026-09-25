#include "pch.h"
#include "FirstStartDialog.h"
#include "app/AppName.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

FirstStartDialog::FirstStartDialog(CWnd* parent) : CDialogEx(IDD_FIRSTSTART, parent)
{
}

bool FirstStartDialog::IsRunAsPortable() const
{
	return mSelectedMode == 1;
}

void FirstStartDialog::DoDataExchange(CDataExchange* dx)
{
	CDialogEx::DoDataExchange(dx);
	DDX_Radio(dx, IDC_RADIO_SAVETOUSERPROFILE, mSelectedMode);
	DDX_Text(dx, IDC_STATIC_SAVEPATH, mSavePathText);
}

BOOL FirstStartDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mSavePathText.Format(_T("ユーザーフォルダ内の %s に保存します。"), (LPCTSTR)APP_PROFILE_DIRNAME);

	UpdateData(FALSE);

	return TRUE;
}

void FirstStartDialog::OnOK()
{
	UpdateData();
	__super::OnOK();
}

