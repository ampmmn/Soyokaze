#include "pch.h"
#include "framework.h"
#include "URLCommand.h"
#include "commands/common/SubProcess.h"
#include "commands/common/Clipboard.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace bookmarks {

static bool GetChromeExecutablePath(LPTSTR path, size_t len)
{
	size_t reqLen = 0;
	_tgetenv_s(&reqLen, path, MAX_PATH_NTFS, _T("PROGRAMFILES"));
	PathAppend(path, _T("Google\\Chrome\\Application\\chrome.exe"));

	return true;
}

static bool GetEdgeExecutablePath(LPTSTR path, size_t len)
{
	size_t reqLen = 0;
#ifndef _WIN64
	_tgetenv_s(&reqLen, path, MAX_PATH_NTFS, _T("ProgramFiles"));
#else
	_tgetenv_s(&reqLen, path, MAX_PATH_NTFS, _T("ProgramFiles(x86)"));
#endif
	PathAppend(path, _T("Microsoft\\Edge\\Application\\msedge.exe"));

	return true;
}


struct URLCommand::PImpl
{
	bool GetExecutablePath(LPTSTR path, size_t len);

	CString mBrowserName;    // ブラウザ種類を表す文字列
	int mType;               // 種別(ブックマーク or 履歴)
	CString mUrl;
};

bool URLCommand::PImpl::GetExecutablePath(LPTSTR path, size_t len)
{
	if (mBrowserName == _T("Chrome")) {
		return GetChromeExecutablePath(path, len);
	}
	else if (mBrowserName == _T("Edge")) {
		return GetEdgeExecutablePath(path, len);
	}
	else {
		return false;
	}
}


URLCommand::URLCommand(
	const CString& browserName,
	int type,
	const CString& name,
	const CString& url
) : 
	AdhocCommandBase(name, name),
	in(std::make_unique<PImpl>())
{
	in->mBrowserName = browserName;
	in->mType = type;
	in->mUrl = url;
}

URLCommand::~URLCommand()
{
}

CString URLCommand::GetGuideString()
{
	return _T("Enter:ブラウザで開く Shift-Enter:URLをクリップボードにコピー");
}

CString URLCommand::GetTypeDisplayName()
{
	static CString TEXT_BOOKMARK((LPCTSTR)IDS_COMMAND_BOOKMARK);
	static CString TEXT_HISTORY((LPCTSTR)IDS_COMMAND_HISTORY);

	CString str;
	str.Format(_T("%s %s"), in->mBrowserName, in->mType == BOOKMARK ? TEXT_BOOKMARK : TEXT_HISTORY);

	return str;
}

BOOL URLCommand::Execute(const Parameter& param)
{
	if (param.GetNamedParamBool(_T("ShiftKeyPressed"))) {
		// URLをクリップボードにコピー
		Clipboard::Copy(in->mUrl);
		return TRUE;
	}

	// URLをブラウザで開く
	TCHAR path[MAX_PATH_NTFS];
	if (in->GetExecutablePath(path, MAX_PATH_NTFS) == false) {
		return FALSE;
	}

	if (PathFileExists(path) == FALSE) {
		CString msg(_T("Browser executable not found."));
		msg += _T("\n");
		msg += path;
		AfxMessageBox(msg);
		return TRUE;
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	exec.Run(path, in->mUrl, process);

	return TRUE;
}

HICON URLCommand::GetIcon()
{
	// ブラウザに応じたアイコンを取得
	TCHAR path[MAX_PATH_NTFS];
	in->GetExecutablePath(path, MAX_PATH_NTFS);
	return IconLoader::Get()->LoadIconFromPath(path);
}

launcherapp::core::Command*
URLCommand::Clone()
{
	return new URLCommand(in->mBrowserName, in->mType, this->mName, in->mUrl);
}

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

