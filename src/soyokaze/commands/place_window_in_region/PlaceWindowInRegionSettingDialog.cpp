#include "pch.h"
#include "PlaceWindowInRegionSettingDialog.h"
#include "commands/place_window_in_region/PlaceWindowInRegionParameter.h"
#include "commands/common/Message.h"
#include "commands/place_window_in_region/RegionIndicatorWindow.h"

#include "hotkey/CommandHotKeyDialog.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/ProcessPath.h"
#include "utility/Accessibility.h"
#include "icon/IconLoader.h"
#include "icon/CaptureIconLabel.h"
#include "commands/validation/CommandEditValidation.h"
#include "resource.h"
#include <map>

using namespace launcherapp::commands::common;
using namespace launcherapp::commands::validation;

namespace launcherapp {
namespace commands {
namespace place_window_in_region {

constexpr UINT TIMERID_INDICATE = 1;

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

	int mWidth;
	int mHeight;
	//
	UINT_PTR mIndicateTimerId{0};

	// ウインドウキャプチャ用アイコン
	CaptureIconLabel mIconLabel;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog(CWnd* parentWnd) : 
	launcherapp::control::SinglePageDialog(IDD_PLACEWINDOWINREGION, parentWnd),
	in(std::make_unique<PImpl>())
{
	SetHelpPageId("PlaceWindowInRegionSetting");
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetName(const CString& name)
{
	in->mParam.mName = name;
	ResetHotKey();
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
	DDX_Text(pDX, IDC_EDIT_X, in->mParam.mPlacement.rcNormalPosition.left);
	DDX_Text(pDX, IDC_EDIT_Y, in->mParam.mPlacement.rcNormalPosition.top);
	DDX_Text(pDX, IDC_EDIT_WIDTH, in->mWidth);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, in->mHeight);
	DDX_Check(pDX, IDC_CHECK_ACTIVEWINDOW, in->mParam.mIsActivate);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, in->mHotKey);
}

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::control::SinglePageDialog)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_COMMAND(IDC_BUTTON_TEST, OnButtonTest)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_CAPTION, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_CLASS, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_REGEXP, OnUpdateStatus)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_MESSAGE(WM_APP+6, OnUserMessageCaptureWindow)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mIconLabel.SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mIconLabel.DrawIcon(IconLoader::Get()->LoadWindowIcon());

	CRect rc = in->mParam.mPlacement.rcNormalPosition;
	in->mWidth = rc.Width();
	in->mHeight = rc.Height();

	CString caption(_T("コマンドの設定"));

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	UpdateStatus();

	in->mHotKey = in->mParam.mHotKeyAttr.ToString();
	if (in->mHotKey.IsEmpty()) {
		in->mHotKey.LoadString(IDS_NOHOTKEY);
	}

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
	if (in->mHotKey.IsEmpty()) {
		in->mHotKey.LoadString(IDS_NOHOTKEY);
	}

	UpdateStatus();

	UpdateData(FALSE);
}

void SettingDialog::OnButtonTest()
{
	UpdateData();

	if (UpdateStatus() == false) {
		return ;
	}

	// 領域を強調表示
	auto window = RegionIndicatorWindow::GetInstance();
	window->SetIndependentMode(true);

	auto& rc = in->mParam.mPlacement.rcNormalPosition;
	window->Cover(rc);

	if (in->mIndicateTimerId != 0) {
		KillTimer(TIMERID_INDICATE);
	}
	in->mIndicateTimerId = SetTimer(TIMERID_INDICATE, 1000, 0);
}

void SettingDialog::OnTimer(UINT_PTR timerId)
{
	if (timerId != TIMERID_INDICATE) {
		return;
	}
	KillTimer(TIMERID_INDICATE);
	in->mIndicateTimerId = 0;

	auto window = RegionIndicatorWindow::GetInstance();
	window->SetIndependentMode(false);
	window->Uncover();
}


void SettingDialog::OnOK()
{
	UpdateData();

	auto window = RegionIndicatorWindow::GetInstance();
	window->SetIndependentMode(false);

	int errCode;
	if (in->mParam.IsValid(in->mOrgName, &errCode)) {

		in->mParam.mPlacement.rcNormalPosition.right = in->mParam.mPlacement.rcNormalPosition.left + in->mWidth;
		in->mParam.mPlacement.rcNormalPosition.bottom = in->mParam.mPlacement.rcNormalPosition.top + in->mHeight;

		__super::OnOK();
		return ;
	}

	CommandParamError paramErr(errCode);
	AfxMessageBox(paramErr.ToString());
}

void SettingDialog::OnCancel()
{
	auto window = RegionIndicatorWindow::GetInstance();
	window->SetIndependentMode(false);

	__super::OnCancel();
}

bool SettingDialog::UpdateStatus()
{
	int errCode;
	bool isValid = in->mParam.IsValid(in->mOrgName, &errCode);

	BOOL isRectEmpty = in->mWidth <= 0 || in->mHeight <= 0;
	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(isRectEmpty);

	// 状態に応じてOKボタンとステータス欄の表示を変える
	if (isValid) {
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(TRUE);
		in->mMessage.Empty();
		return true;
	}
	else {
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		CommandParamError paramErr(errCode);
		in->mMessage = paramErr.ToString();
		return false;
	}
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
	if (::utility::IsHighContrastMode()) {
		return br;
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_STATUSMSG) {
		COLORREF crTxt = in->mMessage.IsEmpty() ? RGB(0,0,0) : RGB(255, 0, 0);
		pDC->SetTextColor(crTxt);
	}
	return br;
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
	::GetWindowPlacement(hwndRoot, &in->mParam.mPlacement);
	CRect rc(in->mParam.mPlacement.rcNormalPosition);
	in->mWidth = rc.Width();
	in->mHeight = rc.Height();

	UpdateStatus();
	UpdateData(FALSE);

	//in->mIconLabel.DrawIcon(IconLoader::Get()->LoadWindowIcon());

	return 0;
}


}
}
}
