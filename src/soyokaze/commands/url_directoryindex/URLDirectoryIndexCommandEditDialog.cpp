#include "pch.h"
#include "framework.h"
#include "URLDirectoryIndexCommandEditDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/validation/CommandEditValidation.h"
#include "commands/common/ExpandFunctions.h"
#include "utility/Accessibility.h"
#include "app/Manual.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CommandRepository = launcherapp::core::CommandRepository;
using Command = launcherapp::core::Command;

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

URLDirectoryIndexCommandEditDialog::URLDirectoryIndexCommandEditDialog(CWnd* parentWnd) : 
	launcherapp::control::SinglePageDialog(IDD_URLDIRECTORYINDEXEDIT, parentWnd)
{
	SetHelpPageId("DirectoryIndexEdit");
}

URLDirectoryIndexCommandEditDialog::~URLDirectoryIndexCommandEditDialog()
{
}

void URLDirectoryIndexCommandEditDialog::SetName(const CString& name)
{
	mParam.mName = name;
}

void URLDirectoryIndexCommandEditDialog::SetOriginalName(const CString& name)
{
	mOrgName = name;
}

void URLDirectoryIndexCommandEditDialog::SetParam(const CommandParam& param)
{
	mParam = param;
}

const CommandParam& URLDirectoryIndexCommandEditDialog::GetParam()
{
	return mParam;
}

void URLDirectoryIndexCommandEditDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_URL, mParam.mURL);
	DDX_Text(pDX, IDC_EDIT_AUTHUSER, mParam.mServerUser);
	DDX_Text(pDX, IDC_EDIT_AUTHPASSWORD, mServerPassword);
	DDX_CBIndex(pDX, IDC_COMBO_PROXY, mParam.mProxyType);
	DDX_Text(pDX, IDC_EDIT_PROXYHOST, mParam.mProxyHost);
	DDX_Text(pDX, IDC_EDIT_PROXYUSER, mParam.mProxyUser);
	DDX_Text(pDX, IDC_EDIT_PROXYPASSWORD, mProxyPassword);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(URLDirectoryIndexCommandEditDialog, launcherapp::control::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_URL, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_PROXYHOST, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_PROXY, OnUpdateStatus)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_APP + 1, OnUserMessageHotKeyChange)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL URLDirectoryIndexCommandEditDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mServerPassword = mParam.DecryptServerPassword();
	mProxyPassword = mParam.DecryptProxyPassword();

	mHotKey.SubclassDlgItem(IDC_EDIT_HOTKEY, this);
	mHotKey.SetNotifyId(WM_APP+1);

	CString caption;
  GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)mOrgName);

	caption += suffix;
	SetWindowText(caption);

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool URLDirectoryIndexCommandEditDialog::UpdateStatus()
{
	mHotKey.UpdateContent(mParam.mHotKeyAttr.ToString());

	// プロキシ欄の有効/無効
	BOOL isEnableProxyHost = mParam.mProxyType == 1;  // 直接指定する
	GetDlgItem(IDC_EDIT_PROXYHOST)->EnableWindow(isEnableProxyHost);
	GetDlgItem(IDC_EDIT_PROXYUSER)->EnableWindow(isEnableProxyHost);
	GetDlgItem(IDC_EDIT_PROXYPASSWORD)->EnableWindow(isEnableProxyHost);

	if (isEnableProxyHost) {
		static tregex reg(_T("^[0-9a-zA-z-.]+:[0-9]+$"));
		if (std::regex_match(tstring((LPCTSTR)mParam.mProxyHost), reg) == false) {
			mMessage = _T("プロキシを「ホスト名:ポート番号」の形式で指定してください");
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			return false;
		}
	}

	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::validation::IsValidCommandName(mParam.mName, mOrgName, mMessage);
	if (isNameValid == false) {
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	//
	if (mParam.mURL.IsEmpty()) {
		mMessage = _T("URLを設定してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	return true;
}

void URLDirectoryIndexCommandEditDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH URLDirectoryIndexCommandEditDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void URLDirectoryIndexCommandEditDialog::OnOK()
{
	UpdateData();
	if (UpdateStatus() == false) {
		return ;
	}

	mParam.SetServerPassword(mServerPassword);
	mParam.SetProxyPassword(mProxyPassword);

	__super::OnOK();
}


LRESULT URLDirectoryIndexCommandEditDialog::OnUserMessageHotKeyChange(WPARAM, LPARAM)
{
	UpdateData();

	if (mHotKey.EditHotKey(mParam.mName, mParam.mHotKeyAttr, this) == false) {
		return 0;
	}
	UpdateStatus();
	UpdateData(FALSE);

	return 0;
}

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

