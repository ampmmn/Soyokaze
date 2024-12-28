#include "pch.h"
#include "WindowActivateAdhocNameDialog.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace activate_window {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



AdhocNameDialog::AdhocNameDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_WINDOWACTIVATEADHOCNAME, parentWnd)
{
	SetHelpPageId(_T("ActivateWindowAdhocName"));
}

const CString& AdhocNameDialog::GetName()
{
	return mName;
}

void AdhocNameDialog::SetName(const CString& name)
{
	mName = name;
}

void AdhocNameDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NAME, mName);
}

BEGIN_MESSAGE_MAP(AdhocNameDialog, launcherapp::gui::SinglePageDialog)
END_MESSAGE_MAP()

void AdhocNameDialog::OnOK()
{
	UpdateData();
	__super::OnOK();
}

}
}
}
