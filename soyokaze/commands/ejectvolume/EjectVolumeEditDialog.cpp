#include "pch.h"
#include "EjectVolumeEditDialog.h"
#include "icon/IconLabel.h"
#include "icon/IconLoader.h"
#include "commands/common/CommandEditValidation.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace ejectvolume {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct SettingDialog::PImpl
{
	void SetIcon(HICON hIcon)
	{
		mIcon = hIcon;
	}

	// 設定情報
	CommandParam mParam;
	int mDriveLetterIndex;

	CString mOrgName;

	// メッセージ欄
	CString mMessage;
	// 選択したドライブレターの情報を表示
	CString mDriveLetterMsg;

	HICON mIcon = nullptr;
	std::unique_ptr<IconLabel> mIconLabelPtr;

	CString mHotKey;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog(CWnd* parentWnd) : 
	launcherapp::gui::SinglePageDialog(IDD_EJECTVOLUMEEDIT, parentWnd),
	in(std::make_unique<PImpl>())
{
	SetHelpPageId(_T("EjectVolumeSetting"));
	in->mIconLabelPtr = std::make_unique<IconLabel>();
}

SettingDialog::~SettingDialog()
{
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

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Text(pDX, IDC_STATIC_DRIVELETTER, in->mDriveLetterMsg);
	DDX_CBIndex(pDX, IDC_COMBO_DRIVELETTER, in->mDriveLetterIndex);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, in->mHotKey);
}

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_VOLUME, OnUpdateStatus)
	ON_CBN_SELCHANGE(IDC_COMBO_DRIVELETTER, OnUpdateStatus)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->SetIcon(IconLoader::Get()->GetImageResIcon(-5328));

	in->mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);

	in->mOrgName = in->mParam.mName;

	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	in->mDriveLetterIndex = (int)(in->mParam.mDriveLetter - _T('A'));

	CString caption;
	GetWindowText(caption);

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	UpdateStatus();
	UpdateData(FALSE);

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	return TRUE;
}

void SettingDialog::UpdateStatus()
{
	in->mMessage.Empty();

	if (in->mIcon) {
		in->mIconLabelPtr->DrawIcon(in->mIcon);
	}

	TCHAR letter = (TCHAR)(in->mDriveLetterIndex + _T('A'));

	LPCTSTR driveTypeText = _T("無効なドライブ");

	TCHAR rootName[] = { letter, _T(':'), _T('\\'), _T('\0') };
	UINT type = GetDriveType(rootName);
	switch(type) {
	case DRIVE_REMOVABLE:
		driveTypeText = _T("リムーバブルメディア");
		break;
	case DRIVE_FIXED:
		driveTypeText = _T("固定ドライブ");
		break;
	case DRIVE_REMOTE:
		driveTypeText = _T("ネットワークドライブ");
		break;
	case DRIVE_CDROM:
		driveTypeText = _T("ディスクドライブ");
		break;
	case DRIVE_UNKNOWN:
		driveTypeText = _T("不明なドライブ");
		break;
	case DRIVE_NO_ROOT_DIR:
	default:
		break;
	}

	in->mDriveLetterMsg.Format(_T("現在のシステムでは%sです"), driveTypeText);
	if (type != DRIVE_NO_ROOT_DIR && type != DRIVE_CDROM && type != DRIVE_REMOVABLE) {
		in->mDriveLetterMsg += _T("\n(取り外しができるのはリムーバブルメディアとディスクドライブのみ)");
	}

	// 名前チェック
	bool canPressOK =
	 	launcherapp::commands::common::IsValidCommandName(in->mParam.mName, in->mOrgName, in->mMessage);

	// OKボタンの状態変更
	GetDlgItem(IDOK)->EnableWindow(canPressOK ? TRUE : FALSE);
}

void SettingDialog::OnOK()
{
	UpdateData();

	in->mParam.mDriveLetter = (TCHAR)(in->mDriveLetterIndex + _T('A'));

	__super::OnOK();
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

void SettingDialog::OnButtonHotKey()
{
	UpdateData();

	if (CommandHotKeyDialog::ShowDialog(in->mParam.mName, in->mParam.mHotKeyAttr, this) == false) {
		return ;
	}
	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	UpdateData(FALSE);
}


}
}
}
