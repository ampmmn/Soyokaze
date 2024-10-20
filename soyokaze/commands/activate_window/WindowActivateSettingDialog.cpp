#include "pch.h"
#include "WindowActivateSettingDialog.h"
#include "commands/activate_window/WindowActivateCommandParam.h"
#include "commands/common/Message.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "icon/CaptureIconLabel.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/ProcessPath.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "commands/common/CommandEditValidation.h"
#include "resource.h"

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace activate_window {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct SettingDialog::PImpl
{
	// 設定情報
	CommandParam mParam;
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

	// ホットキー(表示用)
	CString mHotKey;

	// ウインドウキャプチャ用アイコン
	CaptureIconLabel mIconLabel;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_WINDOWACTIVATEEDIT, parentWnd),
	in(std::make_unique<PImpl>())
{
	SetHelpPageId(_T("ActivateWindowSetting"));
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetName(const CString& name)
{
	in->mParam.mName = name;
}

void SettingDialog::SetOriginalName(const CString& name)
{
	in->mOrgName = name;
}

void SettingDialog::SetParam(const Param& param)
{
	in->mParam = param;
}

const SettingDialog::Param&
SettingDialog::GetParam()
{
	return in->mParam;
}

void SettingDialog::ResetHotKey()
{
	in->mParam.mHotKeyAttr.Reset();
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_CAPTION, in->mParam.mCaptionStr);
	DDX_Text(pDX, IDC_EDIT_CLASS, in->mParam.mClassStr);
	DDX_Check(pDX, IDC_CHECK_REGEXP, in->mParam.mIsUseRegExp);
	DDX_Check(pDX, IDC_CHECK_NOTIFYIFNOTEXIST, in->mParam.mIsNotifyIfWindowNotFound);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, in->mHotKey);
}

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::gui::SinglePageDialog)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_COMMAND(IDC_BUTTON_TEST, OnButtonTest)
	ON_MESSAGE(WM_APP+6, OnUserMessageCaptureWindow)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_CAPTION, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_CLASS, OnUpdateStatus)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mIconLabel.SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mIconLabel.DrawIcon(IconLoader::Get()->LoadWindowIcon());

	CString caption(_T("コマンドの設定"));

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	UpdateStatus();

	UpdateData(FALSE);

	return TRUE;
}

void SettingDialog::OnButtonHotKey()
{
	UpdateData();
	if (CommandHotKeyDialog::ShowDialog(in->mParam.mName, in->mParam.mHotKeyAttr, this) == false) {
		return ;
	}
	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	UpdateData(FALSE);
}

void SettingDialog::OnButtonTest()
{
	UpdateData();

	if (UpdateStatus() == false) {
		return ;
	}

	HWND hwnd = in->mParam.FindHwnd();
	if (IsWindow(hwnd) == FALSE) {
		PopupMessage(_T("ウインドウは見つかりませんでした"));
		return;
	}

	FLASHWINFO fi;
	fi.cbSize = sizeof(fi);
	fi.hwnd = hwnd;
	fi.dwFlags = FLASHW_ALL;
	fi.uCount = 2;
	fi.dwTimeout = 500;
	::FlashWindowEx(&fi);
}

void SettingDialog::OnOK()
{
	UpdateData();
	__super::OnOK();
}

bool SettingDialog::UpdateStatus()
{
	bool canTest = in->mParam.mCaptionStr.IsEmpty() == FALSE || in->mParam.mClassStr.IsEmpty() == FALSE;
	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(canTest);

	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::common::IsValidCommandName(in->mParam.mName, in->mOrgName, in->mMessage);
	if (isNameValid == false) {
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}
	if (canTest == false) {
		in->mMessage = _T("ウインドウタイトルかウインドウクラスを入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	CString msg;
	if (in->mParam.BuildCaptionRegExp(&msg)  == false) {
		AfxMessageBox(msg);
		GetDlgItem(IDC_EDIT_CAPTION)->SetFocus();
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	if (in->mParam.BuildClassRegExp(&msg)  == false) {
		AfxMessageBox(msg);
		GetDlgItem(IDC_EDIT_CLASS)->SetFocus();
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	in->mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	return true;
}

LRESULT
SettingDialog::OnUserMessageCaptureWindow(WPARAM pParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(pParam);

	HWND hTargetWnd = (HWND)lParam;
	if (IsWindow(hTargetWnd) == FALSE) {
		return 0;
	}
	ProcessPath processPath(hTargetWnd);

	// 自プロセスのウインドウなら何もしない
	if (GetCurrentProcessId() == processPath.GetProcessId()) {
		return 0;
	}

	HWND hwndRoot = ::GetAncestor(hTargetWnd, GA_ROOT);

	TCHAR caption[256];
	::GetWindowText(hwndRoot, caption, 256);
	TCHAR clsName[256];
	::GetClassName(hwndRoot, clsName, 256);

	in->mParam.mCaptionStr = caption;
	in->mParam.mClassStr = clsName;
	in->mParam.mIsUseRegExp = FALSE;

	UpdateStatus();
	UpdateData(FALSE);

	try {
		CString path = processPath.GetProcessPath();
		in->mIconLabel.DrawIcon(IconLoader::Get()->GetDefaultIcon(path));
	}
	catch (ProcessPath::Exception&) {
		in->mIconLabel.DrawIcon(IconLoader::Get()->LoadWindowIcon());
	}
	return 0;
}

void SettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH SettingDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = in->mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
}

}
}
}
