#include "pch.h"
#include "AlignWindowItemSettingDialog.h"
#include "commands/common/Message.h"
#include "gui/CaptureIconLabel.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/TopMostMask.h"
#include "utility/ProcessPath.h"
#include "IconLoader.h"
#include "resource.h"

using namespace soyokaze::commands::common;

namespace soyokaze {
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

	TopMostMask mTopMostMask;

	int mWidth;
	int mHeight;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ItemDialog::ItemDialog(CWnd* parentWnd) : 
	CDialogEx(IDD_ALIGNWINDOWITEMEDIT, this),
	in(std::make_unique<PImpl>())
{
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
}

BEGIN_MESSAGE_MAP(ItemDialog, CDialogEx)
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

	HWND hwnd = in->mParam.FindHwnd();
	if (IsWindow(hwnd) == FALSE) {
		AfxMessageBox(_T("ウインドウは見つかりませんでした"));
		return;
	}

	::GetWindowPlacement(hwnd, &in->mParam.mPlacement);

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (style & WS_MAXIMIZE) {
		in->mParam.mAction = CommandParam::AT_MAXIMIZE;
	}
	else if (style & WS_MINIMIZE) {
		in->mParam.mAction = CommandParam::AT_MINIMIZE;
	}
	else {
		in->mParam.mAction = CommandParam::AT_SETPOS;
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
	if (in->mParam.mAction == CommandParam::AT_MAXIMIZE) {
		in->mParam.mPlacement.showCmd = SW_MAXIMIZE;
	}
	else if (in->mParam.mAction == CommandParam::AT_MINIMIZE) {
		in->mParam.mPlacement.showCmd = SW_MINIMIZE;
	}
	else if (in->mParam.mAction == CommandParam::AT_SETPOS) {
		in->mParam.mPlacement.showCmd = SW_RESTORE;
	}
	else if (in->mParam.mAction == CommandParam::AT_HIDE) {
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

	bool isSetPos = in->mParam.mAction != CommandParam::AT_MAXIMIZE && in->mParam.mAction != CommandParam::AT_MINIMIZE && 
	                in->mParam.mAction != CommandParam::AT_HIDE;
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
		in->mParam.mAction = CommandParam::AT_MAXIMIZE;
	}
	else if (style & WS_MINIMIZE) {
		in->mParam.mAction = CommandParam::AT_MINIMIZE;
	}
	else {
		in->mParam.mAction = CommandParam::AT_SETPOS;
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

}
}
}
