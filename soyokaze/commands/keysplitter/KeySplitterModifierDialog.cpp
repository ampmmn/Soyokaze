#include "pch.h"
#include "KeySplitterModifierDialog.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace keysplitter {

ModifierDialog::ModifierDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_KEYSPLITTERMODIFIER, parentWnd),
	mIsPressShift(FALSE),
	mIsPressCtrl(FALSE),
	mIsPressAlt(FALSE),
	mIsPressWin(FALSE)
{
	SetHelpPageId(_T("KeySplitterModifier"));
}

ModifierDialog::~ModifierDialog()
{
}

void ModifierDialog::SetParam(const ModifierState& state)
{
	mIsPressShift = state.IsPressShift();
	mIsPressCtrl = state.IsPressCtrl();
	mIsPressAlt = state.IsPressAlt();
	mIsPressWin = state.IsPressWin();
}

void ModifierDialog::GetParam(ModifierState& state) const
{
	state.SetPressShift(mIsPressShift);
	state.SetPressCtrl(mIsPressCtrl);
	state.SetPressAlt(mIsPressAlt);
	state.SetPressWin(mIsPressWin);
}

void ModifierDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SHIFT, mIsPressShift);
	DDX_Check(pDX, IDC_CHECK_CTRL, mIsPressCtrl);
	DDX_Check(pDX, IDC_CHECK_ALT, mIsPressAlt);
	DDX_Check(pDX, IDC_CHECK_WIN, mIsPressWin);
}

BEGIN_MESSAGE_MAP(ModifierDialog, CDialogEx)
END_MESSAGE_MAP()

BOOL ModifierDialog::OnInitDialog()
{
	__super::OnInitDialog();
	return TRUE;
}

void ModifierDialog::OnOK()
{
	UpdateData();
	__super::OnOK();
}



} // end of namespace keysplitter
} // end of namespace commands
} // end of namespace launcherapp

