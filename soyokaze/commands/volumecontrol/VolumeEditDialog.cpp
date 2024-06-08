#include "pch.h"
#include "VolumeEditDialog.h"
#include "commands/volumecontrol/VolumeCommandParam.h"
#include "icon/IconLabel.h"
#include "icon/IconLoader.h"
#include "commands/common/CommandEditValidation.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace volumecontrol {

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
	CString mOrgName;

	CString mVolumeStr;

	// メッセージ欄
	CString mMessage;

	HICON mIcon = nullptr;
	std::unique_ptr<IconLabel> mIconLabelPtr;

	CString mHotKey;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog() : 
	launcherapp::gui::SinglePageDialog(IDD_VOLUMEEDIT),
	in(std::make_unique<PImpl>())
{
	SetHelpPageId(_T("VolumeSetting"));
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
	DDX_Check(pDX, IDC_CHECK_VOLUME, in->mParam.mIsSetVolume);
	DDX_Text(pDX, IDC_EDIT_VOLUME, in->mVolumeStr);
	DDX_CBIndex(pDX, IDC_COMBO_MUTE, in->mParam.mMuteControl);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, in->mHotKey);
}

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::gui::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_VOLUME, OnUpdateStatus)
	ON_COMMAND(IDC_CHECK_VOLUME, OnUpdateStatus)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->SetIcon(IconLoader::Get()->LoadVolumeIcon(false));

	in->mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);

	in->mOrgName = in->mParam.mName;

	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	// 音量の設定値に合わせてテキスト文字列作成
	if (in->mParam.mIsRelative) {

		LPCTSTR sign = _T("");
		if (in->mParam.mVolume >= 0) {
			sign = _T("+");
		}
		else {
			sign = _T("-");
		}
		in->mVolumeStr.Format(_T("%s%d"), sign, abs(in->mParam.mVolume));
	}
	else {
		in->mVolumeStr.Format(_T("%d"), abs(in->mParam.mVolume));
	}

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

	BOOL isSetVolume = in->mParam.mIsSetVolume;
	GetDlgItem(IDC_EDIT_VOLUME)->EnableWindow(isSetVolume);

	// 名前チェック
	bool canPressOK =
	 	launcherapp::commands::common::IsValidCommandName(in->mParam.mName, in->mOrgName, in->mMessage);

	// 音量値のvalidation
	static tregex regVolume(_T("^ *[+-]?\\d+ *$"));
	if (std::regex_match(tstring(in->mVolumeStr), regVolume) == false) {
		in->mMessage = _T("音量は数値で指定してください");
		canPressOK = false;
	}

	// OKボタンの状態変更
	GetDlgItem(IDOK)->EnableWindow(canPressOK ? TRUE : FALSE);
}

void SettingDialog::OnOK()
{
	UpdateData();

	tregex regVolume(_T("^ *([+-]?)(\\d+) *$"));

	if (std::regex_match(tstring(in->mVolumeStr), regVolume) == false) {
		return;
	}

	tstring sign = std::regex_replace(tstring(in->mVolumeStr), regVolume, _T("$1"));
	tstring value = std::regex_replace(tstring(in->mVolumeStr), regVolume, _T("$2"));

	int level =_ttoi(value.c_str());
	if (100 < level) {
		level = 100;
	}

	// 符号があったら相対指定
	bool hasSign = (sign.empty() == false);
	in->mParam.mIsRelative = hasSign;

	if (sign == _T("-")) {
		level = -level;
	}
	in->mParam.mVolume = level;


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

	CommandHotKeyDialog dlg(in->mParam.mHotKeyAttr, in->mParam.mIsGlobal);
	dlg.SetTargetName(in->mParam.mName);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(in->mParam.mHotKeyAttr);
	in->mParam.mIsGlobal = dlg.IsGlobal();
	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	UpdateData(FALSE);
}


}
}
}
