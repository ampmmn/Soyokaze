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
	DDX_Text(pDX, IDC_EDIT_X, in->mParam.mPos.x);
	DDX_Text(pDX, IDC_EDIT_Y, in->mParam.mPos.y);
	DDX_Text(pDX, IDC_EDIT_WIDTH, in->mParam.mSize.cx);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, in->mParam.mSize.cy);
	DDX_Check(pDX, IDC_CHECK_REGEXP, in->mParam.mIsUseRegExp);
}

BEGIN_MESSAGE_MAP(ItemDialog, CDialogEx)
	ON_COMMAND(IDC_BUTTON_UPDATE, OnButtonUpdate)
	ON_MESSAGE(WM_APP+6, OnUserMessageCaptureWindow)
	ON_EN_CHANGE(IDC_EDIT_CAPTION, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_CLASS, OnUpdateStatus)
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
		PopupMessage(_T("ウインドウは見つかりませんでした"));
		return;
	}

	// サイズを取得
	CRect rcWindow;
	::GetWindowRect(hwnd, &rcWindow);
	in->mParam.mPos.x = rcWindow.left;
	in->mParam.mPos.y = rcWindow.top;
	in->mParam.mSize.cx = rcWindow.Width();
	in->mParam.mSize.cy = rcWindow.Height();

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

	__super::OnOK();
}

bool ItemDialog::UpdateStatus()
{
	bool canTest = in->mParam.mCaptionStr.IsEmpty() == FALSE || in->mParam.mClassStr.IsEmpty() == FALSE;
	GetDlgItem(IDC_BUTTON_UPDATE)->EnableWindow(canTest);

	if (canTest == false) {
		in->mMessage = _T("ウインドウタイトルかウインドウクラスを入力してください");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		return false;
	}

	in->mMessage.Empty();
	GetDlgItem(IDOK)->EnableWindow(TRUE);
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

	// サイズを取得
	CRect rcWindow;
	::GetWindowRect(hwndRoot, &rcWindow);
	in->mParam.mPos.x = rcWindow.left;
	in->mParam.mPos.y = rcWindow.top;
	in->mParam.mSize.cx = rcWindow.Width();
	in->mParam.mSize.cy = rcWindow.Height();

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
