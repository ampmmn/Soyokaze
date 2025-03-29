#include "pch.h"
#include "framework.h"
#include "OtherCommandDialog.h"
#include "commands/core/CommandRepository.h"
#include "utility/Accessibility.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using Command = launcherapp::core::Command;

namespace launcherapp {
namespace commands {
namespace common {


OtherCommandDialog::OtherCommandDialog(LPCTSTR helpId, CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_FILTER_AFTER_COMMAND, parentWnd),
	mCommandSelIndex(-1)
{
	SetHelpPageId(helpId);
	//SetHelpPageId(_T("PostFilterCommand"));
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
	DDX_CBIndex(pDX, IDC_COMBO_AFTERCOMMAND, mCommandSelIndex);
	DDX_Text(pDX, IDC_EDIT_PARAM2, mParam.mCommandParam);
	DDX_Text(pDX, IDC_STATIC_VARIABLE, mVariableText);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(OtherCommandDialog, launcherapp::gui::SinglePageDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_AFTERCOMMAND, OnUpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL OtherCommandDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// コマンド一覧のコンボボックス
	std::vector<Command*> commands;
	auto cmdRepo = CommandRepository::GetInstance();
	cmdRepo->EnumCommands(commands);

	CComboBox* commandComboBox =
	 	(CComboBox*)GetDlgItem(IDC_COMBO_AFTERCOMMAND);
	ASSERT(commandComboBox);

	for (auto& cmd : commands) {
		CString name = cmd->GetName();
		int idx = commandComboBox->AddString(name);
		cmd->Release();

		if (name == mParam.mCommandName) {
			mCommandSelIndex = idx;
		}
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool OtherCommandDialog::UpdateStatus()
{
	if (mCommandSelIndex == -1) {
		mMessage = _T("絞込み後に実行するコマンドを選んでください");
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
	if (utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

void OtherCommandDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	CComboBox* cmbBox = (CComboBox*)GetDlgItem(IDC_COMBO_AFTERCOMMAND);
	cmbBox->GetLBText(mCommandSelIndex, mParam.mCommandName);

	__super::OnOK();
}

} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

