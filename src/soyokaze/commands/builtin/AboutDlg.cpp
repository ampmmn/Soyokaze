#include "pch.h"
#include "framework.h"
#include "app/AppName.h"
#include "AboutDlg.h"
#include "utility/AppProfile.h"
#include "utility/VersionInfo.h"
#include "externaltool/webbrowser/ConfiguredBrowserEnvironment.h"
#include "resource.h"
#include <vector>

#pragma comment(lib, "version.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ConfiguredBrowserEnvironment = launcherapp::externaltool::webbrowser::ConfiguredBrowserEnvironment;

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



