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
	std::map<String, CString> mPageMapping;
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

bool Manual::Navigate(const char* pageId)
{
	SPDLOG_DEBUG("start");

	// ヘルプの有無を確認
	Path filePath(Path::MODULEFILEDIR);
	filePath.Append(L"files\\manuals\\index.html");
	if (filePath.FileExists() == FALSE) {
		CString msg((LPCTSTR)IDS_ERR_HELPDOESNOTEXIST);
		msg += _T("\n");
		msg += filePath;
		AfxMessageBox(msg);

		SPDLOG_WARN(L"help file does not exist. filePath:{}", (LPCWSTR)filePath);
		return false;
	}

	// クッションページ?のパス生成
	Path dirPath(Path::MODULEFILEDIR);
	dirPath.Append(L"files\\fragment");
	SPDLOG_DEBUG(L"dirPath:{}", (LPCWSTR)dirPath);

	// 指定されたIDに対応するページの有無を確認
	CString pagePath;
	auto it = in->mPageMapping.find(pageId);
	if (it != in->mPageMapping.end()) {
		pagePath = it->second;
	}
	else {
		UTF2UTF(pageId, pagePath);
	}

	CStringW uri;
	uri.Format(L"file:///%s/%s.html", (LPCWSTR)dirPath, (LPCWSTR)pagePath);
	uri.Replace(L'\\', L'/');

	auto p = uri.GetBuffer(uri.GetLength() + 1);

	SHELLEXECUTEINFOW si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = p;

	ShellExecuteExW(&si);

	uri.ReleaseBuffer();

	if (si.hProcess == nullptr) {
		SPDLOG_WARN(L"Failed to launch help: {}", (LPCWSTR)pagePath);
	}
	else {
		SPDLOG_DEBUG(L"launch help PID:{}", GetProcessId(si.hProcess));
		CloseHandle(si.hProcess);
	}

	return true;
}

}
}
