#include "pch.h"
#include "framework.h"
#include "SnippetCommandEditDialog.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "commands/validation/CommandEditValidation.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace snippet {


CommandEditDialog::CommandEditDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_SNIPPET_EDIT, parentWnd)
{
	SetHelpPageId("SnippetEdit");
}

CommandEditDialog::~CommandEditDialog()
{
}

void CommandEditDialog::SetName(const CString& name)
{
	mParam.mName = name;
}

void CommandEditDialog::SetOriginalName(const CString& name)
{
	mOrgName = name;
}

void CommandEditDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

const CommandParam& CommandEditDialog::GetParam()
{
	return mParam;
}

void CommandEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_TEXT, mParam.mText);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, mHotKey);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CommandEditDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_TEXT, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
END_MESSAGE_MAP()

#pragma warning( pop )

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
	mHotKey = mParam.mHotKeyAttr.ToString();
	if (mHotKey.IsEmpty()) {
		mHotKey.LoadString(IDS_NOHOTKEY);
	}

	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::validation::IsValidCommandName(mParam.mName, mOrgName, mMessage);
	if (isNameValid == false) {
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	if (mParam.mText.IsEmpty()) {
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
	if (::utility::IsHighContrastMode()) {
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

	if (CommandHotKeyDialog::ShowDialog(mParam.mName, mParam.mHotKeyAttr, this) == false) {
		return ;
	}
	UpdateStatus();
	UpdateData(FALSE);
}

// マニュアル表示
void CommandEditDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate("MacroList");
	*pResult = 0;
}

}
}
}

