#include "pch.h"
#include "Manual.h"
#include "resource.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace app {

struct Manual::PImpl
{
	std::map<CString, CString> mPageMapping;
};


Manual::Manual() : in(new PImpl)
{
	// 仮
	in->mPageMapping[_T("Top")] = _T("help");
}

Manual::~Manual()
{
}

Manual* Manual::GetInstance()
{
	static Manual inst;
	return &inst;
}

bool Manual::Navigate(const CString& pageId)
{
	SPDLOG_DEBUG(_T("start"));

	// ヘルプの有無を確認
	TCHAR dirPath[MAX_PATH_NTFS];
	GetModuleFileName(NULL, dirPath, MAX_PATH_NTFS);
	PathRemoveFileSpec(dirPath);

	TCHAR filePath[MAX_PATH_NTFS];
	_tcscpy_s(filePath, dirPath);
	PathAppend(filePath, _T("help.html"));
	if (PathFileExists(filePath) == FALSE) {
		CString msg((LPCTSTR)IDS_ERR_HELPDOESNOTEXIST);
		msg += _T("\n");
		msg += filePath;
		AfxMessageBox(msg);

		SPDLOG_WARN(_T("help file does not exist. filePath:{}"), (LPCTSTR)filePath);
		return false;
	}

	// クッションページ?のパス生成
	PathAppend(dirPath, _T("files"));
	PathAppend(dirPath, _T("fragment"));
	SPDLOG_DEBUG(_T("dirPath:{}"), (LPCTSTR)dirPath);

	// 指定されたIDに対応するページの有無を確認
	CString pagePath;
	auto it = in->mPageMapping.find(pageId);
	if (it != in->mPageMapping.end()) {
		pagePath = it->second;
	}
	else {
		pagePath = pageId;
	}

	CString uri;
	uri.Format(_T("file:///%s/%s.html"), dirPath, pagePath);
	uri.Replace(_T('\\'), _T('/'));

	auto p = uri.GetBuffer(uri.GetLength() + 1);

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = p;

	ShellExecuteEx(&si);

	uri.ReleaseBuffer();

	SPDLOG_DEBUG(_T("launch help PID:{}"), GetProcessId(si.hProcess));

	CloseHandle(si.hProcess);

	return true;
}

}
}
