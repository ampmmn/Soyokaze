#include "pch.h"
#include "framework.h"
#include "SnippetCommandEditDialog.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "commands/core/CommandRepository.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace snippet {


CommandEditDialog::CommandEditDialog() : 
	CDialogEx(IDD_SNIPPET_EDIT)
{
}

CommandEditDialog::~CommandEditDialog()
{
}

void CommandEditDialog::SetOrgName(const CString& name)
{
	mOrgName = name;
}

void CommandEditDialog::SetName(const CString& name)
{
	mName = name;
}

void CommandEditDialog::SetDescription(const CString& desc)
{
	mDescription = desc;
}

void CommandEditDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mDescription);
	DDX_Text(pDX, IDC_EDIT_TEXT, mText);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, mHotKey);
}

BEGIN_MESSAGE_MAP(CommandEditDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_TEXT, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


BOOL CommandEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	SetIcon(IconLoader::Get()->LoadDefaultIcon(), FALSE);

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)mOrgName);

	caption += suffix;
	SetWindowText(caption);

	UpdateStatus();
	UpdateData(FALSE);

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	return TRUE;
}

bool CommandEditDialog::UpdateStatus()
{
	mHotKey = mHotKeyAttr.ToString();
	if (mHotKey.IsEmpty()) {
		mHotKey.LoadString(IDS_NOHOTKEY);
	}

	if (mName.IsEmpty()) {
		mMessage.LoadString(IDS_ERR_NAMEISEMPTY);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();

	// 重複チェック
	if (mName.CompareNoCase(mOrgName) != 0) {
		auto cmd = cmdRepoPtr->QueryAsWholeMatch(mName, false);
		if (cmd != nullptr) {
			cmd->Release();
			mMessage.LoadString(IDS_ERR_NAMEALREADYEXISTS);
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			return false;
		}
	}

	// 使えない文字チェック
	if (cmdRepoPtr->IsValidAsName(mName) == false) {
		mMessage.LoadString(IDS_ERR_ILLEGALCHARCONTAINS);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	if (mText.IsEmpty()) {
		mMessage.LoadString(IDS_ERR_TEXTISEMPTY);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}


	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	return true;
}

void CommandEditDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH CommandEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CommandEditDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	__super::OnOK();
}

void CommandEditDialog::OnButtonHotKey()
{
	UpdateData();

	CommandHotKeyDialog dlg(mHotKeyAttr);
	dlg.mIsGlobal = mIsGlobal;
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttr);
	mIsGlobal = dlg.IsGlobal();

	UpdateStatus();
	UpdateData(FALSE);
}

}
}
}

