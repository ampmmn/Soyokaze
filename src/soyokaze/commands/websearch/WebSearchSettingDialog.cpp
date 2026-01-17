#include "pch.h"
#include "WebSearchSettingDialog.h"
#include "commands/websearch/WebSearchCommandParam.h"
#include "control/KeywordEdit.h"
#include "icon/IconLabel.h"
#include "icon/IconLoader.h"
#include "commands/validation/CommandEditValidation.h"
#include "hotkey/CommandHotKeyDialog.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Accessibility.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace websearch {

struct SITE_ITEM {
	int mID;
	LPCTSTR mSiteName;
	LPCTSTR mURL;
};

static std::vector<SITE_ITEM> SITE_TEMPLATE = {
	{ 1, _T("&Google"), _T("https://www.google.com/search?q=$*") },
	{ 2, _T("&Bing"), _T("https://www.bing.com/search?q=$*") },
	{ 3, _T("&DuckDuckGo"), _T("https://duckduckgo.com/?t=h_&q=$*") },
	{ 4, _T("&X"), _T("https://x.com/search?q=$*") },
	{ 5, _T("&Amazon"), _T("https://www.amazon.co.jp/s?k=$*") },
	{ 6, _T("&Youtube"), _T("https://www.youtube.com/results?search_query=$*") },
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct SettingDialog::PImpl
{

	void SetIcon(HICON hIcon)
	{
		mIcon = hIcon;
	}

	void InitSiteMenu()
	{
		mMenuForSiteBtn.CreatePopupMenu();
		for (auto& item : SITE_TEMPLATE) {
			mMenuForSiteBtn.InsertMenu((UINT)-1, 0, item.mID, item.mSiteName);
		}
		mSiteMenuBtn.m_hMenu = (HMENU)mMenuForSiteBtn;
	}

	// 設定情報
	CommandParam mParam;
	CString mOrgName;

	int mIsEnableShortcut{false};

	// メッセージ欄
	CString mMessage;

	KeywordEdit mQueryEdit;

	HICON mIcon{nullptr};
	std::unique_ptr<IconLabel> mIconLabelPtr;

	// URLの例
	CMFCMenuButton mSiteMenuBtn;
	CMenu mMenuForSiteBtn;

	CString mHotKey;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog(CWnd* parentWnd) : 
	launcherapp::control::SinglePageDialog(IDD_WEBSEARCHEDIT, parentWnd),
	in(std::make_unique<PImpl>())
{
	SetHelpPageId("WebSearchSetting");
	in->mIconLabelPtr = std::make_unique<IconLabel>();
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
	DDX_Text(pDX, IDC_EDIT_HOTKEY, in->mHotKey);
	DDX_Control(pDX, IDC_BUTTON_MENU, in->mSiteMenuBtn);
}

BEGIN_MESSAGE_MAP(SettingDialog, launcherapp::control::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnUpdateStatus)
	ON_EN_CHANGE(IDC_EDIT_URL, OnUpdateStatus)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_MESSAGE(WM_APP + 11, OnUserMessageIconChanged)
	ON_BN_CLICKED(IDC_BUTTON_MENU, OnSiteMenuBtnClicked)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->InitSiteMenu();

	in->mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mIconLabelPtr->EnableIconChange();

	in->mQueryEdit.SubclassDlgItem(IDC_EDIT_URL, this);
	in->mQueryEdit.SetNotifyKeyEvent(false);
	in->mQueryEdit.SetPlaceHolder(_T("例:https://www.google.com/search?q=$*"));

	in->mHotKey = in->mParam.mHotKeyAttr.ToString();
	if (in->mHotKey.IsEmpty()) {
		in->mHotKey.LoadString(IDS_NOHOTKEY);
	}

	CString caption(_T("Web検索コマンドの設定"));

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	in->mIsEnableShortcut = in->mParam.mIsEnableShortcut;

	if (in->mParam.mIconData.empty() == false) {
		SetIcon(IconLoader::Get()->LoadIconFromStream(in->mParam.mIconData));
	}

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

	static tregex regHttp(_T("^https?://.+$"));

	CString& url = in->mParam.mURL;

	bool canPressOK = true;

	bool isNameValid =
	 	launcherapp::commands::validation::IsValidCommandName(in->mParam.mName, in->mOrgName, in->mMessage);
	if (isNameValid == false) {
		canPressOK = false;
	}
	else if (url.IsEmpty()) {
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
	if (::utility::IsHighContrastMode()) {
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
	if (in->mHotKey.IsEmpty()) {
		in->mHotKey.LoadString(IDS_NOHOTKEY);
	}

	UpdateData(FALSE);
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

void SettingDialog::OnSiteMenuBtnClicked()
{
	UpdateData();

	int id = in->mSiteMenuBtn.m_nMenuResult;
	for (const auto& item : SITE_TEMPLATE) {
		if (id != item.mID) {
			continue;
		}
		in->mParam.mURL = item.mURL;
		in->mParam.mDescription = item.mSiteName;
		in->mParam.mDescription.Replace(_T("&"),_T(""));

		UpdateStatus();
		UpdateData(FALSE);
		break;
	}
}


}
}
}
