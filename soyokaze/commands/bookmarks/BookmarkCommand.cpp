#include "pch.h"
#include "framework.h"
#include "BookmarkCommand.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
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


struct BookmarkCommand::PImpl
{
	bool GetExecutablePath(LPTSTR path, size_t len);

	CString mType;    // ブラウザ種類を表す文字列
	CString mUrl;
};

bool BookmarkCommand::PImpl::GetExecutablePath(LPTSTR path, size_t len)
{
	if (mType == _T("Chrome")) {
		return GetChromeExecutablePath(path, len);
	}
	else if (mType == _T("Edge")) {
		return GetEdgeExecutablePath(path, len);
	}
	else {
		return false;
	}
}


BookmarkCommand::BookmarkCommand(
	const CString& type,
	const CString& name,
	const CString& url
) : 
	AdhocCommandBase(name, type + _T(" - ") + name),
	in(std::make_unique<PImpl>())
{
	in->mType = type;
	in->mUrl = url;
}

BookmarkCommand::~BookmarkCommand()
{
}

CString BookmarkCommand::GetTypeDisplayName()
{
	static CString TEXT_BOOKMARK((LPCTSTR)IDS_COMMAND_BOOKMARK);

	if (in->mType == _T("Chrome") || in->mType == _T("Edge")) {
		return in->mType + _T(" ") + TEXT_BOOKMARK;
	}
	else {
		return TEXT_BOOKMARK;
	}
}

BOOL BookmarkCommand::Execute(const Parameter& param)
{
	TCHAR path[MAX_PATH_NTFS];
	if (in->GetExecutablePath(path, MAX_PATH_NTFS) == false) {
		return FALSE;
	}

	if (PathFileExists(path) == FALSE) {
		CString msg(_T("Browser executable does not found."));
		msg += _T("\n");
		msg += path;
		AfxMessageBox(msg);
		return TRUE;
	}

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;
	si.lpParameters = in->mUrl;

	ShellExecuteEx(&si);
	CloseHandle(si.hProcess);

	return TRUE;
}

HICON BookmarkCommand::GetIcon()
{
	// ブラウザに応じたアイコンを取得
	TCHAR path[MAX_PATH_NTFS];
	in->GetExecutablePath(path, MAX_PATH_NTFS);
	return IconLoader::Get()->LoadIconFromPath(path);
}

soyokaze::core::Command*
BookmarkCommand::Clone()
{
	return new BookmarkCommand(in->mType, this->mName, in->mUrl);
}

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace soyokaze

