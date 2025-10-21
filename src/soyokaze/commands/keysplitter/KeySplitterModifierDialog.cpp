#include "pch.h"
#include "KeySplitterModifierDialog.h"
#include "gui/DDXWrapper.h"
#include "commands/core/CommandRepository.h"
#include "utility/Accessibility.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using Command = launcherapp::core::Command;

namespace launcherapp {
namespace commands {
namespace keysplitter {

ModifierDialog::ModifierDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_KEYSPLITTERMODIFIER, parentWnd)
{
	SetHelpPageId("KeySplitterEdit");
}

ModifierDialog::~ModifierDialog()
{
}

void ModifierDialog::SetParam(const CommandParam& param)
{
	mParamPtr = &param;
}

void ModifierDialog::SetItem(const ITEM& item)
{
	mItem = item;
}

void ModifierDialog::GetItem(ITEM& item)
{
	item = mItem;
}

void ModifierDialog::SetModifierState(const ModifierState& state)
{
	mOrgState = state;
	mState = state;
}

void ModifierDialog::GetModifierState(ModifierState& state) const
{
	state = mState;
}

void ModifierDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_COMMANDS, mCommandSelIndex);
	DDX_Text(pDX, IDC_EDIT_NAME, mItem.mActionName);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Check(pDX, IDC_CHECK_SHIFT, mState.mIsPressShift);
	DDX_Check(pDX, IDC_CHECK_CTRL, mState.mIsPressCtrl);
	DDX_Check(pDX, IDC_CHECK_ALT, mState.mIsPressAlt);
	DDX_Check(pDX, IDC_CHECK_WIN, mState.mIsPressWin);
}

BEGIN_MESSAGE_MAP(ModifierDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_CHECK_SHIFT, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_CTRL, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_ALT, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_WIN, OnUpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


bool ModifierDialog::UpdateStatus()
{
	if (mState != mOrgState && mParamPtr->IsStateExists(mState)) {
		mMessage = _T("既に定義されています");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	return true;
}

BOOL ModifierDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// コマンド一覧のコンボボックス
	std::vector<Command*> commands;
	auto cmdRepo = CommandRepository::GetInstance();
	cmdRepo->EnumCommands(commands);

	auto commandComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMANDS);
	ASSERT(commandComboBox);

	for (auto& cmd : commands) {
		CString name = cmd->GetName();
		int idx = commandComboBox->AddString(name);
		cmd->Release();

		if (name == mItem.mCommandName) {
			mCommandSelIndex = idx;
		}
	}

	UpdateData(FALSE);


	return TRUE;
}

void ModifierDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void ModifierDialog::OnOK()
{
	UpdateData();

	auto commandComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMANDS);
	ASSERT(commandComboBox);
	commandComboBox->GetLBText(commandComboBox->GetCurSel(), mItem.mCommandName);

	__super::OnOK();
}

HBRUSH ModifierDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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



} // end of namespace keysplitter
} // end of namespace commands
} // end of namespace launcherapp

