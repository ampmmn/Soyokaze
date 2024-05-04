#include "pch.h"
#include "SinglePageDialog.h"
#include "resource.h"
#include "app/Manual.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace gui {


SinglePageDialog::SinglePageDialog(UINT resId, CWnd* parent) : CDialogEx(resId, parent),
	mAccel(nullptr)
{
	ACCEL accels[1];
	accels[0].cmd = ID_VIEW_HELP;
	accels[0].fVirt = FVIRTKEY;
	accels[0].key = 0x70;   // F1
	mAccel = CreateAcceleratorTable(accels, 1);
}

SinglePageDialog::~SinglePageDialog()
{
	if (mAccel) {
		DestroyAcceleratorTable(mAccel);
	}
}

BEGIN_MESSAGE_MAP(SinglePageDialog, CDialogEx)
	ON_WM_NCLBUTTONDOWN()
	ON_COMMAND(ID_VIEW_HELP, OnCommandHelp)
END_MESSAGE_MAP()

void SinglePageDialog::SetHelpPageId(const CString& id)
{
	mHelpPageId = id;
}

bool SinglePageDialog::ShowHelp()
{
	if (mHelpPageId.IsEmpty()) {
		SPDLOG_WARN(_T("Help id is empty."));
		return false;
	}
	return launcherapp::app::Manual::GetInstance()->Navigate(mHelpPageId);
}


BOOL SinglePageDialog::OnInitDialog()
{
	__super::OnInitDialog();

	ModifyStyleEx(0, WS_EX_CONTEXTHELP);

	return TRUE;
}


BOOL SinglePageDialog::PreTranslateMessage(MSG* pMsg)
{
	if (mAccel && TranslateAccelerator(GetSafeHwnd(), mAccel, pMsg)) {
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}


void SinglePageDialog::OnNcLButtonDown(UINT nHitTest, CPoint pt)
{
	if (nHitTest == HTHELP) {
		ShowHelp();
	}
	else {
		__super::OnNcLButtonDown(nHitTest, pt);
	}
}

void SinglePageDialog::OnCommandHelp()
{
	ShowHelp();
}


}
}

