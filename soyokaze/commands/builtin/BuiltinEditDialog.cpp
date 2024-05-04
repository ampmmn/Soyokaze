#include "pch.h"
#include "BuiltinEditDialog.h"
#include "commands/core/CommandRepository.h"
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

	bool mCanEditEnable;
	bool mCanEditConfirm;

	BOOL mIsEnable;
	BOOL mIsConfirm;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



BuiltinEditDialog::BuiltinEditDialog(const CString& name, bool canEditEnable, bool canEditConfirm) : 
	launcherapp::gui::SinglePageDialog(IDD_BUILTINEDIT),
	in(std::make_unique<PImpl>())
{
	SetHelpPageId(_T("BuiltinCommandEdit"));
	in->mName = name;
	in->mCanEditEnable = canEditEnable;
	in->mCanEditConfirm = canEditConfirm;
}

BuiltinEditDialog::~BuiltinEditDialog()
{
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
	DDX_Check(pDX, IDC_CHECK_ENABLE, in->mIsEnable);
	DDX_Check(pDX, IDC_CHECK_CONFIRM, in->mIsConfirm);
}

BEGIN_MESSAGE_MAP(BuiltinEditDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_CHECK_ENABLE, OnUpdateStatus)
END_MESSAGE_MAP()


BOOL BuiltinEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	GetDlgItem(IDC_CHECK_ENABLE)->EnableWindow(in->mCanEditEnable ? TRUE: FALSE);
	GetDlgItem(IDC_CHECK_CONFIRM)->EnableWindow(in->mCanEditConfirm ? TRUE: FALSE);

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

	GetDlgItem(IDC_CHECK_CONFIRM)->EnableWindow(in->mIsEnable);
}

}
}
}
