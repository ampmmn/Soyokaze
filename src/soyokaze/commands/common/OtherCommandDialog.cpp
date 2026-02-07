#include "pch.h"
#include "framework.h"
#include "OtherCommandDialog.h"
#include "commands/common/CommandSelectDialog.h"
#include "utility/Accessibility.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandSelectDialog = launcherapp::commands::common::CommandSelectDialog;

namespace launcherapp {
namespace commands {
namespace common {


OtherCommandDialog::OtherCommandDialog(LPCSTR helpId, CWnd* parentWnd) : 
	launcherapp::control::SinglePageDialog(IDD_FILTER_AFTER_COMMAND, parentWnd)
{
	SetHelpPageId(helpId);
}

OtherCommandDialog::~OtherCommandDialog()
{
}

void OtherCommandDialog::SetParam(const Param& param)
{
	mParam = param;
}

const OtherCommandDialog::Param&
OtherCommandDialog::GetParam()
{
	return mParam;
}

void OtherCommandDialog::SetVariableDescription(LPCTSTR text)
{
	mVariableText = text;
}


void OtherCommandDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mCommandName);
	DDX_Text(pDX, IDC_EDIT_PARAM2, mParam.mCommandParam);
	DDX_Text(pDX, IDC_STATIC_VARIABLE, mVariableText);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(OtherCommandDialog, launcherapp::control::SinglePageDialog)
	ON_COMMAND(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL OtherCommandDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool OtherCommandDialog::UpdateStatus()
{
	if (mParam.mCommandName.IsEmpty()) {
		mMessage = _T("コマンドを選択してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	return true;
}

void OtherCommandDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH OtherCommandDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void OtherCommandDialog::OnButtonBrowse()
{
	CommandSelectDialog dlg(this);
	dlg.SetCommandName(mParam.mCommandName);
	if (dlg.DoModal() != IDOK) {
		return;
	}
	mParam.mCommandName = dlg.GetCommandName();

	UpdateStatus();
	UpdateData(FALSE);
}


void OtherCommandDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}
	__super::OnOK();
}

} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

