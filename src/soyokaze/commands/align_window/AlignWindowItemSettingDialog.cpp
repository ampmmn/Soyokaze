#include "pch.h"
#include "AlignWindowItemSettingDialog.h"
#include "commands/common/Message.h"
#include "icon/CaptureIconLabel.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/ProcessPath.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "resource.h"

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace align_window {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct ItemDialog::PImpl
{
	// 設定情報
	Param mParam;

	// メッセージ欄
	CString mMessage;

	// ウインドウキャプチャ用アイコン
	CaptureIconLabel mIconLabel;

	int mWidth{0};
	int mHeight{0};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ItemDialog::ItemDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_ALIGNWINDOWITEMEDIT, parentWnd),
	in(std::make_unique<PImpl>())
{
	UNREFERENCED_PARAMETER(parentWnd);

	SetHelpPageId("AlignWindowItemSetting");
}

ItemDialog::~ItemDialog()
{
}

void ItemDialog::SetParam(const Param& param)
{
	in->mParam = param;
}

const ItemDialog::Param&
ItemDialog::GetParam()
{
	return in->mParam;
}

void ItemDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_CAPTION, in->mParam.mCaptionStr);
	DDX_Text(pDX, IDC_EDIT_CLASS, in->mParam.mClassStr);
	DDX_Text(pDX, IDC_EDIT_X, in->mParam.mPlacement.rcNormalPosition.left);
	DDX_Text(pDX, IDC_EDIT_Y, in->mParam.mPlacement.rcNormalPosition.top);
	DDX_Text(pDX, IDC_EDIT_WIDTH, in->mWidth);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, in->mHeight);
	DDX_Check(pDX, IDC_CHECK_REGEXP, in->mParam.mIsUseRegExp);
	DDX_CBIndex(pDX, IDC_COMBO_ACTION, in->mParam.mAction);
	DDX_Check(pDX, IDC_CHECK_APPLYALL, in->mParam.mIsApplyAll);
}

BEGIN_MESSAGE_MAP(ItemDialog, launcherapp::gui::SinglePageDialog)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_UPDATE, OnButtonUpdate)
	ON_MESSAGE(WM_APP+6, OnUserMessageCaptureWindow)
	ON_EN_CHANGE(IDC_EDIT_CAPTION, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_CLASS, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_ACTION, OnUpdateStatus)
END_MESSAGE_MAP()

BOOL ItemDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mIconLabel.SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mIconLabel.DrawIcon(IconLoader::Get()->LoadWindowIcon());

	CString caption(_T("整列対象ウインドウの設定"));
	SetWindowText(caption);

	UpdateStatus();

	UpdateData(FALSE);

	return TRUE;
}

void ItemDialog::OnButtonUpdate()
{
	UpdateData();

	if (UpdateStatus() == false) {
		return ;
	}

	std::vector<HWND> targets;
	in->mParam.FindHwnd(targets);
	if (targets.empty()) {
		AfxMessageBox(_T("ウインドウは見つかりませんでした"));
		return;
	}

	HWND hwnd = targets[0];
	::GetWindowPlacement(hwnd, &in->mParam.mPlacement);

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (style & WS_MAXIMIZE) {
		in->mParam.mAction = ACTION::AT_MAXIMIZE;
	}
	else if (style & WS_MINIMIZE) {
		in->mParam.mAction = ACTION::AT_MINIMIZE;
	}
	else {
		in->mParam.mAction = ACTION::AT_SETPOS;
	}


	UpdateStatus();

	UpdateData(FALSE);
}

void ItemDialog::OnOK()
{
	UpdateData();

	// 正規表現のテスト
	CString msg;
	if (in->mParam.BuildCaptionRegExp(&msg)  == false) {
		AfxMessageBox(msg);
		GetDlgItem(IDC_EDIT_CAPTION)->SetFocus();
		return ;
	}

	if (in->mParam.BuildClassRegExp(&msg)  == false) {
		AfxMessageBox(msg);
		GetDlgItem(IDC_EDIT_CLASS)->SetFocus();
		return ;
	}

	// ダイアログを閉じるタイミングでmActionの値に応じてWINDOWPLACEMENT.showCmdを書き換える
	if (in->mParam.mAction == ACTION::AT_MAXIMIZE) {
		in->mParam.mPlacement.showCmd = SW_MAXIMIZE;
	}
	else if (in->mParam.mAction == ACTION::AT_MINIMIZE) {
		in->mParam.mPlacement.showCmd = SW_MINIMIZE;
	}
	else if (in->mParam.mAction == ACTION::AT_SETPOS) {
		in->mParam.mPlacement.showCmd = SW_RESTORE;
	}
	else if (in->mParam.mAction == ACTION::AT_HIDE) {
		in->mParam.mPlacement.showCmd = SW_HIDE;
	}
	__super::OnOK();
}

bool ItemDialog::UpdateStatus()
{
	bool canTest = in->mParam.mCaptionStr.IsEmpty() == FALSE || in->mParam.mClassStr.IsEmpty() == FALSE;
	GetDlgItem(IDC_BUTTON_UPDATE)->EnableWindow(canTest);

	if (canTest == false) {
		in->mMessage = _T("ウインドウタイトルかウインドウクラスを入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_ACTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_X)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_Y)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_WIDTH)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_HEIGHT)->EnableWindow(FALSE);
		return false;
	}

	CString msg;
	if (in->mParam.BuildCaptionRegExp(&msg)  == false) {
		AfxMessageBox(msg);
		GetDlgItem(IDC_EDIT_CAPTION)->SetFocus();
		return false;
	}

	if (in->mParam.BuildClassRegExp(&msg)  == false) {
		AfxMessageBox(msg);
		GetDlgItem(IDC_EDIT_CLASS)->SetFocus();
		return false;
	}

	in->mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMBO_ACTION)->EnableWindow(TRUE);

	bool isSetPos = in->mParam.mAction != ACTION::AT_MAXIMIZE && in->mParam.mAction != ACTION::AT_MINIMIZE && 
	                in->mParam.mAction != ACTION::AT_HIDE;
	GetDlgItem(IDC_EDIT_X)->EnableWindow(isSetPos);
	GetDlgItem(IDC_EDIT_Y)->EnableWindow(isSetPos);
	GetDlgItem(IDC_EDIT_WIDTH)->EnableWindow(isSetPos);
	GetDlgItem(IDC_EDIT_HEIGHT)->EnableWindow(isSetPos);

	auto& rc = in->mParam.mPlacement.rcNormalPosition;
	in->mWidth = rc.right - rc.left;
	in->mHeight = rc.bottom - rc.top;

	return true;
}

LRESULT
ItemDialog::OnUserMessageCaptureWindow(WPARAM pParam, LPARAM lParam)
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

	::GetWindowPlacement(hwndRoot, &in->mParam.mPlacement);

	LONG_PTR style = GetWindowLongPtr(hwndRoot, GWL_STYLE);
	if (style & WS_MAXIMIZE) {
		in->mParam.mAction = ACTION::AT_MAXIMIZE;
	}
	else if (style & WS_MINIMIZE) {
		in->mParam.mAction = ACTION::AT_MINIMIZE;
	}
	else {
		in->mParam.mAction = ACTION::AT_SETPOS;
	}

	UpdateStatus();
	UpdateData(FALSE);

	return 0;
}

void ItemDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

HBRUSH ItemDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH br = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if (::utility::IsHighContrastMode()) {
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
