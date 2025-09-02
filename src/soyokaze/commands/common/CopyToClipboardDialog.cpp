#include "pch.h"
#include "framework.h"
#include "CopyToClipboardDialog.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {


CopyToClipboardDialog::CopyToClipboardDialog(LPCSTR helpId, CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_FILTER_AFTER_CLIPBOARD, parentWnd)
{
	SetHelpPageId(helpId);
}

CopyToClipboardDialog::~CopyToClipboardDialog()
{
}

void CopyToClipboardDialog::SetParam(const Param& param)
{
	mParam = param;
}

const CopyToClipboardDialog::Param&
CopyToClipboardDialog::GetParam()
{
	return mParam;
}

void CopyToClipboardDialog::SetVariableDescription(LPCTSTR text)
{
	mVariableText = text;
}

void CopyToClipboardDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PARAM2, mParam.mCommandParam);
	DDX_Text(pDX, IDC_STATIC_VARIABLE, mVariableText);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CopyToClipboardDialog, launcherapp::gui::SinglePageDialog)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL CopyToClipboardDialog::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool CopyToClipboardDialog::UpdateStatus()
{
	return true;
}

void CopyToClipboardDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void CopyToClipboardDialog::OnOK()
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

