#include "pch.h"
#include "WebHistoryAdhocCommand.h"
#include "commands/common/SubProcess.h"
#include "commands/common/Clipboard.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include "framework.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace webhistory {

static bool GetChromeExecutablePath(LPTSTR path, size_t len)
{
	UNREFERENCED_PARAMETER(len);

	size_t reqLen = 0;
	_tgetenv_s(&reqLen, path, MAX_PATH_NTFS, _T("PROGRAMFILES"));
	PathAppend(path, _T("Google\\Chrome\\Application\\chrome.exe"));

	return true;
}

static bool GetEdgeExecutablePath(LPTSTR path, size_t len)
{
	UNREFERENCED_PARAMETER(len);

	size_t reqLen = 0;
#ifndef _WIN64
	_tgetenv_s(&reqLen, path, MAX_PATH_NTFS, _T("ProgramFiles"));
#else
	_tgetenv_s(&reqLen, path, MAX_PATH_NTFS, _T("ProgramFiles(x86)"));
#endif
	PathAppend(path, _T("Microsoft\\Edge\\Application\\msedge.exe"));

	return true;
}

constexpr LPCTSTR TYPENAME = _T("WebHistoryAdhocCommand");

struct WebHistoryAdhocCommand::PImpl
{
	bool GetExecutablePath(LPTSTR path, size_t len);

	HISTORY mHistory;
};

bool WebHistoryAdhocCommand::PImpl::GetExecutablePath(LPTSTR path, size_t len)
{
	if (mHistory.mBrowserName == _T("Chrome")) {
		return GetChromeExecutablePath(path, len);
	}
	else if (mHistory.mBrowserName == _T("Edge")) {
		return GetEdgeExecutablePath(path, len);
	}
	else {
		return false;
	}
}


WebHistoryAdhocCommand::WebHistoryAdhocCommand(
	const HISTORY& item
) : 
	AdhocCommandBase(item.mDisplayName, item.mDisplayName),
	in(std::make_unique<PImpl>())
{
	in->mHistory = item;
}

WebHistoryAdhocCommand::~WebHistoryAdhocCommand()
{
}

CString WebHistoryAdhocCommand::GetGuideString()
{
	return _T("Enter:ブラウザで開く Shift-Enter:URLをクリップボードにコピー");
}

/**
 * 種別を表す文字列を取得する
 * @return 文字列
 */
CString WebHistoryAdhocCommand::GetTypeName()
{
	return TYPENAME;
}

CString WebHistoryAdhocCommand::GetTypeDisplayName()
{
	static CString TEXT_HISTORY((LPCTSTR)IDS_COMMAND_HISTORY);

	CString str;
	str.Format(_T("%s %s"), (LPCTSTR)in->mHistory.mBrowserName, (LPCTSTR)TEXT_HISTORY);

	return str;
}

BOOL WebHistoryAdhocCommand::Execute(const Parameter& param)
{
	if (param.GetNamedParamBool(_T("ShiftKeyPressed"))) {
		// URLをクリップボードにコピー
		Clipboard::Copy(in->mHistory.mUrl);
		return TRUE;
	}

	// URLをブラウザで開く
	std::vector<TCHAR> path(MAX_PATH_NTFS);
	if (in->GetExecutablePath(path.data(), MAX_PATH_NTFS) == false) {
		return FALSE;
	}

	if (PathFileExists(path.data()) == FALSE) {
		CString msg(_T("Browser executable not found."));
		msg += _T("\n");
		msg += path.data();
		AfxMessageBox(msg);
		return TRUE;
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	exec.Run(path.data(), in->mHistory.mUrl, process);

	return TRUE;
}

HICON WebHistoryAdhocCommand::GetIcon()
{
	// ブラウザに応じたアイコンを取得
	std::vector<TCHAR> path(MAX_PATH_NTFS);
	in->GetExecutablePath(path.data(), MAX_PATH_NTFS);
	return IconLoader::Get()->LoadIconFromPath(path.data());
}

launcherapp::core::Command*
WebHistoryAdhocCommand::Clone()
{
	return new WebHistoryAdhocCommand(in->mHistory);
}

} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

