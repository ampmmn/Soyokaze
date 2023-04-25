
#include "pch.h"
#include "framework.h"
#include "AboutDlg.h"
#include "resource.h"
#include <vector>

#pragma comment(lib, "version.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_VERSION, m_strVersion);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog()
{
	__super::OnInitDialog();

	// バージョン情報を取得
	TCHAR szModulePath[65536];
	GetModuleFileName( NULL, szModulePath, 65536);

	DWORD size = GetFileVersionInfoSize(szModulePath, NULL);
	std::vector<BYTE> versionData(size);

	BYTE* pVersion = &(versionData.front());
	if (GetFileVersionInfo(szModulePath, NULL, size, pVersion)) {

		UINT actualLen;
		VS_FIXEDFILEINFO* pFileInfo;
		VerQueryValue(pVersion, _T("\\"), (void**)&pFileInfo, &actualLen);

		m_strVersion.Format(_T("%d.%d.%d"), HIWORD(pFileInfo->dwFileVersionMS), LOWORD(pFileInfo->dwFileVersionMS), HIWORD(pFileInfo->dwFileVersionLS));
	}

	UpdateData(FALSE);

	return TRUE;
}
