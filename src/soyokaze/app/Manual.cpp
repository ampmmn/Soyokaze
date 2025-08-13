#include "pch.h"
#include "Manual.h"
#include "utility/Path.h"
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
	Path filePath(Path::MODULEFILEDIR);
	filePath.Append(_T("files\\manuals\\index.html"));
	if (filePath.FileExists() == FALSE) {
		CString msg((LPCTSTR)IDS_ERR_HELPDOESNOTEXIST);
		msg += _T("\n");
		msg += filePath;
		AfxMessageBox(msg);

		SPDLOG_WARN(_T("help file does not exist. filePath:{}"), (LPCTSTR)filePath);
		return false;
	}

	// クッションページ?のパス生成
	Path dirPath(Path::MODULEFILEDIR);
	dirPath.Append(_T("files\\fragment"));
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
	uri.Format(_T("file:///%s/%s.html"), (LPCTSTR)dirPath, (LPCTSTR)pagePath);
	uri.Replace(_T('\\'), _T('/'));

	auto p = uri.GetBuffer(uri.GetLength() + 1);

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = p;

	ShellExecuteEx(&si);

	uri.ReleaseBuffer();

	if (si.hProcess == nullptr) {
		SPDLOG_WARN(_T("Failed to launch help: {}"), (LPCTSTR)pagePath);
	}
	else {
		SPDLOG_DEBUG(_T("launch help PID:{}"), GetProcessId(si.hProcess));
		CloseHandle(si.hProcess);
	}

	return true;
}

}
}
