#include "pch.h"
#include "framework.h"
#include "AfterCopyDialog.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace filter {


AfterCopyDialog::AfterCopyDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_FILTER_AFTER_CLIPBOARD, parentWnd)
{
	SetHelpPageId(_T("PostFilterCopy"));
}

AfterCopyDialog::~AfterCopyDialog()
{
}

void AfterCopyDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

const CommandParam& AfterCopyDialog::GetParam()
{
	return mParam;
}

void AfterCopyDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PARAM2, mParam.mAfterCommandParam);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AfterCopyDialog, launcherapp::gui::SinglePageDialog)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL AfterCopyDialog::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool AfterCopyDialog::UpdateStatus()
{
	return true;
}

void AfterCopyDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void AfterCopyDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}
	mParam.mPostFilterType = 2;

	__super::OnOK();
}

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

