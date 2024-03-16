#include "pch.h"
#include "framework.h"
#include "app/AppName.h"
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

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_VERSION, mVersionStr);
	DDX_Text(pDX, IDC_STATIC_BUILDDATE, mBuildDateStr);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK1, OnNotifyLinkOpen)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK2, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK2, OnNotifyLinkOpen)
END_MESSAGE_MAP()


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
	parts->SetWindowText(str);


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

		mVersionStr.Format(_T("%d.%d.%d"), HIWORD(pFileInfo->dwFileVersionMS), LOWORD(pFileInfo->dwFileVersionMS), HIWORD(pFileInfo->dwFileVersionLS));
	}

	CTime tmBuildDate;
	GetBuildDateTime(tmBuildDate);

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

	ShellExecute(0, _T("open"), linkPtr->item.szUrl,  0, 0,SW_NORMAL);
	*pResult = 0;
}


bool CAboutDlg::GetBuildDateTime(CTime& tmBuildDate)
{
	TCHAR path[MAX_PATH_NTFS];
	GetModuleFileName(NULL, path, MAX_PATH_NTFS);

	FILE* fp = nullptr;
 	if(_tfopen_s(&fp, path, _T("rb"))!= 0) {
		return false;
	}


	IMAGE_DOS_HEADER hdr;
	if (fread(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
		fclose(fp);
		return false;
	}

	if (hdr.e_lfanew < sizeof(hdr)) {
		fclose(fp);
		return false;
	}

	size_t offset2PE = hdr.e_lfanew - sizeof(hdr);

	uint8_t skipbytes[65536];
	while(offset2PE > 0) {
		size_t skipSize = offset2PE > 65536 ? 65536 : offset2PE;
		if (fread(skipbytes, 1, skipSize, fp) != skipSize) {
			fclose(fp);
			return false;
		}
		offset2PE -= skipSize;
	}

	uint8_t bytes[4];
	if (fread(bytes, 1, 4, fp) != 4) {
		fclose(fp);
		return false;
	}
	if (bytes[0] != 'P' || bytes[1] != 'E' || bytes[2] != '\0'  || bytes[3] != '\0') {
		fclose(fp);
		return false;
	}

	// COFF File Headerからタイムスタンプを読み取る
	uint8_t skipBytes[4];
	if (fread(skipBytes, 1, 4, fp) != 4) {
		fclose(fp);
		return false;
	}

	uint8_t timeStamp[4];
	if (fread(timeStamp, 1, 4, fp) != 4) {
		fclose(fp);
		return false;
	}
	fclose(fp);

	__time64_t offset = *(uint32_t*)timeStamp;

	tmBuildDate = CTime(offset);

	return true;
}

