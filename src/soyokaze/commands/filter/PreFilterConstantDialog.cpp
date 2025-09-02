#include "pch.h"
#include "framework.h"
#include "PreFilterConstantDialog.h"
#include "utility/Accessibility.h"
#include "app/Manual.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace filter {

PreFilterConstantDialog::PreFilterConstantDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_FILTER_PRE_CONSTANT, parentWnd)
{
	SetHelpPageId("PreFilterConstant");
}

PreFilterConstantDialog::~PreFilterConstantDialog()
{
}

void PreFilterConstantDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

const CommandParam& PreFilterConstantDialog::GetParam()
{
	return mParam;
}

void PreFilterConstantDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_VALUE, mParam.mPreFilterText);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(PreFilterConstantDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_VALUE, OnUpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL PreFilterConstantDialog::OnInitDialog()
{
	__super::OnInitDialog();
	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void PreFilterConstantDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == FALSE) {
		return ;
	}

	mParam.mPreFilterType = FILTER_CONSTANT;

	__super::OnOK();
}

bool PreFilterConstantDialog::UpdateStatus()
{
	if (mParam.mPreFilterText.IsEmpty()) {
		mMessage = _T("候補の一覧を入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	return true;
}

void PreFilterConstantDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH PreFilterConstantDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

