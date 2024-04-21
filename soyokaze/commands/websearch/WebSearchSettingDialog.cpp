#include "pch.h"
#include "WebSearchSettingDialog.h"
#include "commands/websearch/WebSearchCommandParam.h"
#include "icon/IconLabel.h"
#include "icon/IconLoader.h"
#include "commands/core/CommandRepository.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/TopMostMask.h"
#include "utility/Accessibility.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace websearch {

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

	int mIsEnableShortcut;

	// メッセージ欄
	CString mMessage;

	HICON mIcon = nullptr;
	std::unique_ptr<IconLabel> mIconLabelPtr;

	TopMostMask mTopMostMask;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog() : 
	CDialogEx(IDD_WEBSEARCHEDIT),
	in(std::make_unique<PImpl>())
{
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

void SettingDialog::SetIcon(HICON icon)
{
	in->SetIcon(icon);
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_URL, in->mParam.mURL);
	DDX_Check(pDX, IDC_CHECK_ENABLESHORTCUT, in->mIsEnableShortcut);
}

BEGIN_MESSAGE_MAP(SettingDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_URL, OnUpdateStatus)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_APP + 11, OnUserMessageIconChanged)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mIconLabelPtr->EnableIconChange();

	in->mOrgName = in->mParam.mName;

	CString caption(_T("Web検索コマンドの設定"));

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	in->mIsEnableShortcut = in->mParam.mIsEnableShortcut;

	UpdateStatus();
	UpdateData(FALSE);

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	return TRUE;
}

void SettingDialog::UpdateStatus()
{
	if (in->mIcon) {
		in->mIconLabelPtr->DrawIcon(in->mIcon);
	}

	tregex regHttp(_T("^https?://.+$"));

	CString& url = in->mParam.mURL;

	bool canPressOK = true;
	if (url.IsEmpty()) {
		in->mMessage = _T("URLを入力してください");
		canPressOK = false;
	}
	else if (std::regex_match(tstring(url), regHttp) == false) {
		in->mMessage = _T("URLは http:// か https:// で始まる必要があります");
		canPressOK = false;
	}
	else {
		in->mMessage.Empty();
	}

	const CString& name = in->mParam.mName;
	if (name.IsEmpty()) {
		in->mMessage.LoadString(IDS_ERR_NAMEISEMPTY);
		canPressOK = false;
	}
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();

	// 重複チェック
	if (name.CompareNoCase(in->mOrgName) != 0) {
		auto cmd = cmdRepoPtr->QueryAsWholeMatch(name, false);
		if (cmd != nullptr) {
			cmd->Release();
			in->mMessage.LoadString(IDS_ERR_NAMEALREADYEXISTS);
			canPressOK = false;
		}
	}
	// 使えない文字チェック
	if (cmdRepoPtr->IsValidAsName(name) == false) {
		in->mMessage.LoadString(IDS_ERR_ILLEGALCHARCONTAINS);
		canPressOK = false;
	}

	if (url.Find(_T("$*")) == -1) {
		GetDlgItem(IDC_CHECK_ENABLESHORTCUT)->ShowWindow(SW_HIDE);
	}
	else {
		GetDlgItem(IDC_CHECK_ENABLESHORTCUT)->ShowWindow(SW_SHOW);
	}

	GetDlgItem(IDOK)->EnableWindow(canPressOK ? TRUE : FALSE);
}

void SettingDialog::OnOK()
{
	UpdateData();

	const CString& url = in->mParam.mURL;
	if (url.Find(_T("$*")) == -1) {
		in->mIsEnableShortcut = FALSE;
	}

	in->mParam.mIsEnableShortcut = (in->mIsEnableShortcut != FALSE);


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

LRESULT SettingDialog::OnUserMessageIconChanged(WPARAM wp, LPARAM lp)
{
	if (wp != 0) {
		// 変更
		LPCTSTR iconPath = (LPCTSTR)lp;
		if (IconLoader::GetStreamFromPath(iconPath, in->mParam.mIconData) == false) {
			AfxMessageBox(_T("指定されたファイルは有効なイメージファイルではありません"));
			return 0;
		}

		SetIcon(IconLoader::Get()->LoadIconFromStream(in->mParam.mIconData));
	}
	else {
		// デフォルトに戻す
		SetIcon(IconLoader::Get()->LoadWebIcon());
		in->mParam.mIconData.clear();
	}

	// 再描画
	if (in->mIcon) {
		in->mIconLabelPtr->DrawIcon(in->mIcon);
	}

	return 0;
}

}
}
}
