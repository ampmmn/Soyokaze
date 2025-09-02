#include "pch.h"
#include "BuiltinEditDialog.h"
#include "commands/core/CommandRepository.h"
#include "utility/Accessibility.h"
#include "utility/ScopeAttachThreadInput.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace builtin {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct BuiltinEditDialog::PImpl
{
	CString mName;
	CString mDescription;

	CString mOrgName;

	bool mCanEditEnable{false};
	bool mCanEditConfirm{false};

	BOOL mIsEnable{FALSE};
	BOOL mIsConfirm{FALSE};

	CString mMessage;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



BuiltinEditDialog::BuiltinEditDialog(
	const CString& name,
	const CString& description,
	bool canEditEnable,
	bool canEditConfirm,
	CWnd* parentWnd
) : 
	launcherapp::gui::SinglePageDialog(IDD_BUILTINEDIT, parentWnd),
	in(std::make_unique<PImpl>())
{
	SetHelpPageId("BuiltinCommandEdit");
	in->mName = name;
	in->mDescription = description;
	in->mCanEditEnable = canEditEnable;
	in->mCanEditConfirm = canEditConfirm;
}

BuiltinEditDialog::~BuiltinEditDialog()
{
}

CString BuiltinEditDialog::GetName()
{
	return in->mName;
}

void BuiltinEditDialog::SetName(LPCTSTR name)
{
	in->mName = name;
}

void BuiltinEditDialog::SetOriginalName(LPCTSTR name)
{
	in->mOrgName = name;
}

void BuiltinEditDialog::SetEnable(bool isEnable)
{
	in->mIsEnable = isEnable ? TRUE : FALSE;
}

bool BuiltinEditDialog::GetEnable()
{
	return in->mIsEnable != FALSE;
}

void BuiltinEditDialog::SetConfirm(bool isConfirm)
{
	in->mIsConfirm = isConfirm ? TRUE : FALSE;
}

bool BuiltinEditDialog::GetConfirm()
{
	return in->mIsConfirm != FALSE;
}

void BuiltinEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mDescription);
	DDX_Check(pDX, IDC_CHECK_ENABLE, in->mIsEnable);
	DDX_Check(pDX, IDC_CHECK_CONFIRM, in->mIsConfirm);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
}

BEGIN_MESSAGE_MAP(BuiltinEditDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ENABLE, OnUpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


BOOL BuiltinEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	GetDlgItem(IDC_CHECK_ENABLE)->EnableWindow(in->mCanEditEnable ? TRUE: FALSE);
	if (in->mCanEditConfirm == false) {
		GetDlgItem(IDC_CHECK_CONFIRM)->ShowWindow(SW_HIDE);
	}

	OnUpdateStatus();

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	return TRUE;
}

void BuiltinEditDialog::OnOK()
{
	UpdateData();
	__super::OnOK();
}

void BuiltinEditDialog::OnUpdateStatus()
{
	UpdateData();

	GetDlgItem(IDOK)->EnableWindow(FALSE);

	GetDlgItem(IDC_CHECK_CONFIRM)->EnableWindow(in->mIsEnable && in->mCanEditConfirm);

	if (in->mName.IsEmpty()) {
		in->mMessage.LoadString(IDS_ERR_NAMEISEMPTY);
		UpdateData(FALSE);
		return;
	}
	// コマンド名の重複チェック
	if (in->mName.CompareNoCase(in->mOrgName) != 0) {
		auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
		RefPtr<launcherapp::core::Command> cmd(cmdRepoPtr->QueryAsWholeMatch(in->mName, false));
		if (cmd != nullptr) {
			in->mMessage.LoadString(IDS_ERR_NAMEALREADYEXISTS);
			UpdateData(FALSE);
			return;
		}
	}
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	in->mMessage.Empty();
	UpdateData(FALSE);
}

HBRUSH BuiltinEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (::utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = in->mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}


}
}
}
