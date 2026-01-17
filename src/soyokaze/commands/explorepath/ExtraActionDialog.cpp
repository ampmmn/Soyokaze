#include "pch.h"
#include "ExtraActionDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/validation/CommandEditValidation.h"
#include "hotkey/ExtraActionHotKeyDialog.h"
#include "utility/Accessibility.h"
#include "resource.h"

using Command = launcherapp::core::Command;

namespace launcherapp { namespace commands { namespace explorepath {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



ExtraActionDialog::ExtraActionDialog(CWnd* parentWnd) : 
	launcherapp::control::SinglePageDialog(IDD_EXTRAACTION_FOR_EXPLOREPATH, parentWnd)
{
	SetHelpPageId("ExtraActionForExplorePath");
}

const ExtraActionDialog::Entry&
ExtraActionDialog::GetEntry()
{
	return mEntry;
}

void ExtraActionDialog::SetEntry(const Entry& entry)
{
	mEntry = entry;
}

void ExtraActionDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_LABEL, mEntry.mLabel);
	DDX_CBIndex(pDX, IDC_COMBO_COMMAND, mCommandIdx);
	DDX_Text(pDX, IDC_EDIT_HOTKEY2, mHotKey);
	DDX_CBIndex(pDX, IDC_COMBO_TARGET, mTargetIdx);
}

BEGIN_MESSAGE_MAP(ExtraActionDialog, launcherapp::control::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_LABEL, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_COMMAND, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL ExtraActionDialog::OnInitDialog()
{
	__super::OnInitDialog();

	if (mEntry.mIsForFile && mEntry.mIsForFolder) {
		mTargetIdx = 2;
	}
	else if (mEntry.mIsForFolder) {
		mTargetIdx = 1;
	}
	else {
		mTargetIdx = 0;
	}

	CComboBox* commandComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMAND);
	ASSERT(commandComboBox);

	// コンボボックスの要素挿入と、mCommandIdxを決定する
	mCommandIdx = -1;
	std::vector<Command*> commands;
	launcherapp::core::CommandRepository::GetInstance()->EnumCommands(commands);
	for (auto& cmd : commands) {

		auto name = cmd->GetName();
		int n = commandComboBox->AddString(name);

		if (name == mEntry.mCommand) {
			mCommandIdx = n;
		}

		// 後始末
		cmd->Release();
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ExtraActionDialog::UpdateStatus()
{
	GetDlgItem(IDOK)->EnableWindow(FALSE);

	mHotKey = mEntry.mHotkeyAttr.ToString();
	if (mHotKey.IsEmpty()) {
		mHotKey.LoadString(IDS_NOHOTKEY);
	}

	if (mEntry.mLabel.IsEmpty()) {
		mMessage = _T("ラベルを入力してください");
		return false;
	}
	if (mEntry.mHotkeyAttr.IsValid() == false) {
		mMessage = _T("ホットキーを設定してください");
		return false;
	}
	if (mCommandIdx== -1) {
		mMessage = _T("コマンドを選択してください");
		return false;
	}

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	return true;
}

void ExtraActionDialog::OnOK()
{
	UpdateData();

	if (UpdateData() == false) {
		return;
	}

	if (mTargetIdx == 2) {
		mEntry.mIsForFile = true;
		mEntry.mIsForFolder = true;
	}
	else if (mTargetIdx == 1) {
		mEntry.mIsForFile = false;
		mEntry.mIsForFolder = true;
	}
	else {
		mEntry.mIsForFile = true;
		mEntry.mIsForFolder = false;
	}

	// mCommandIdxからコマンド名を取得し、mEntry.mCommandに設定
	CComboBox* commandComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_COMMAND);
	ASSERT(commandComboBox);
	commandComboBox->GetLBText(mCommandIdx, mEntry.mCommand);

	__super::OnOK();
}

HBRUSH ExtraActionDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void ExtraActionDialog::OnButtonHotKey()
{
	UpdateData();

	if (ExtraActionHotKeyDialog::ShowDialog(mEntry.mHotkeyAttr, this) == false) {
		return ;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

void ExtraActionDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}



}}}
