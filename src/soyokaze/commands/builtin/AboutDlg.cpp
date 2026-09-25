#include "pch.h"
#include "framework.h"
#include "app/AppName.h"
#include "AboutDlg.h"
#include "utility/AppProfile.h"
#include "utility/VersionInfo.h"
#include "utility/UpdateInfo.h"
#include "utility/WinHttp.h"
#include "externaltool/webbrowser/ConfiguredBrowserEnvironment.h"
#include "resource.h"
#include <vector>

#pragma comment(lib, "version.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ConfiguredBrowserEnvironment = launcherapp::externaltool::webbrowser::ConfiguredBrowserEnvironment;
using launcherapp::utility::IsVersionNewer;
using launcherapp::utility::ParseUpdateInfo;
using launcherapp::utility::ParseVersionNumber;
using launcherapp::utility::UpdateInfo;
using launcherapp::utility::VersionNumber;
using launcherapp::WinHttp;

namespace {

const TCHAR UPDATE_INFO_URL[] = _T("https://ampmmn.github.io/version/soyokaze/update.json");

/**
  SysLinkに設定するURLをマークアップ用にエスケープする
  @param[in] url URL文字列
  @return エスケープ後のURL文字列
*/
CString EscapeSysLinkUrl(const CString& url)
{
	CString escapedUrl(url);
	escapedUrl.Replace(_T("&"), _T("&amp;"));
	escapedUrl.Replace(_T("\""), _T("&quot;"));
	escapedUrl.Replace(_T("<"), _T("&lt;"));
	escapedUrl.Replace(_T(">"), _T("&gt;"));
	return escapedUrl;
}

} // namespace

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_VERSION, mVersionStr);
	DDX_Text(pDX, IDC_STATIC_BUILDDATE, mBuildDateStr);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK1, OnNotifyLinkOpen)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK2, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK2, OnNotifyLinkOpen)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1_UPDATE, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK1_UPDATE, OnNotifyLinkOpen)
	ON_BN_CLICKED(IDC_BUTTON_CHECKUPDATE, OnButtonCheckUpdate)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL CAboutDlg::OnInitDialog()
{
	__super::OnInitDialog();

	// 文字を置換
	CString str;
	GetWindowText(str);
	str.Replace(_T("$APPNAME"), APPNAME);
	SetWindowText(str);

	CWnd* parts = GetDlgItem(IDC_STATIC_HEADER);
	ASSERT(parts);
	parts->GetWindowText(str);
	str.Replace(_T("$APPNAME"), APPNAME);
	parts->SetWindowText(str);

	parts = GetDlgItem(IDC_STATIC_APPNAME);
	ASSERT(parts);
	parts->GetWindowText(str);
	str.Replace(_T("$APPNAME"), APPNAME);

	if (CAppProfile::IsRunAsPortable()) {
		str += _T(" (ポータブル版)");
	}

	parts->SetWindowText(str);

	// リソースの初期状態によらず、表示状態はコードから明示的に設定する
	GetDlgItem(IDC_BUTTON_CHECKUPDATE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_UPDATEMSG)->ShowWindow(SW_SHOW);
	CWnd* updateLink = GetDlgItem(IDC_SYSLINK1_UPDATE);
	updateLink->SetWindowText(_T(""));
	updateLink->ShowWindow(SW_SHOW);

	// バージョン情報を取得
	VersionInfo::GetVersionInfo(mVersionStr);
	// ビルド日時
	CTime tmBuildDate;
	VersionInfo::GetBuildDateTime(tmBuildDate);
	mBuildDateStr = tmBuildDate.Format(_T("%F %T"));

	UpdateData(FALSE);

	return TRUE;
}

void CAboutDlg::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	NMLINK* linkPtr = (NMLINK*)pNMHDR;

	// アプリ設定の 外部ツール > Webブラウザ の設定でURLを開く
	auto brwsEnv = ConfiguredBrowserEnvironment::GetInstance();
	brwsEnv->OpenURL(linkPtr->item.szUrl);

	*pResult = 0;
}

/**
  最新バージョンを確認して結果をダイアログに表示する
*/
void CAboutDlg::OnButtonCheckUpdate()
{
	CWnd* button = GetDlgItem(IDC_BUTTON_CHECKUPDATE);
	CWnd* message = GetDlgItem(IDC_STATIC_UPDATEMSG);
	CWnd* updateLink = GetDlgItem(IDC_SYSLINK1_UPDATE);
	if (button == nullptr || message == nullptr || updateLink == nullptr) {
		return;
	}

	button->EnableWindow(FALSE);
	message->SetWindowText(_T("更新を確認しています..."));
	updateLink->SetWindowText(_T(""));

	UpdateInfo updateInfo;
	std::vector<BYTE> content;
	WinHttp http;
	http.SetTimeout(5000);
	if (http.LoadBinaryContent(UPDATE_INFO_URL, content) == false || ParseUpdateInfo(content, updateInfo) == false) {
		message->SetWindowText(_T("更新を確認できませんでした。"));
		button->EnableWindow(TRUE);
		return;
	}

	CString currentVersionStr;
	VersionNumber currentVersion;
	if (VersionInfo::GetVersionInfo(currentVersionStr) == false ||
		ParseVersionNumber(currentVersionStr, currentVersion) == false) {
		message->SetWindowText(_T("更新を確認できませんでした。"));
		button->EnableWindow(TRUE);
		return;
	}

	if (IsVersionNewer(updateInfo.mVersion, currentVersion) == false) {
		message->SetWindowText(_T("利用中のバージョンは最新です。"));
		button->EnableWindow(TRUE);
		return;
	}

	CString messageText;
	if (updateInfo.mDate.IsEmpty()) {
		messageText = _T("更新版がリリースされています。");
	}
	else {
		messageText.Format(_T("更新版がリリースされています。(更新日: %s)"), (LPCTSTR)updateInfo.mDate);
	}
	message->SetWindowText(messageText);

	CString escapedUrl = EscapeSysLinkUrl(updateInfo.mUrl);
	CString linkMarkup;
	linkMarkup.Format(_T("<A HREF=\"%s\">リリースページを開く</A>"), (LPCTSTR)escapedUrl);
	updateLink->SetWindowText(linkMarkup);

	button->EnableWindow(TRUE);
}



