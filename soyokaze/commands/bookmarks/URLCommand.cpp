#include "pch.h"
#include "framework.h"
#include "URLCommand.h"
#include "commands/common/SubProcess.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "utility/Path.h"
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
	UNREFERENCED_PARAMETER(len);

	size_t reqLen = 0;
	_tgetenv_s(&reqLen, path, len, _T("PROGRAMFILES"));
	PathAppend(path, _T("Google\\Chrome\\Application\\chrome.exe"));

	return true;
}

static bool GetEdgeExecutablePath(LPTSTR path, size_t len)
{
	UNREFERENCED_PARAMETER(len);

	size_t reqLen = 0;
#ifndef _WIN64
	_tgetenv_s(&reqLen, path, len, _T("ProgramFiles"));
#else
	_tgetenv_s(&reqLen, path, len, _T("ProgramFiles(x86)"));
#endif
	PathAppend(path, _T("Microsoft\\Edge\\Application\\msedge.exe"));

	return true;
}

struct URLCommand::PImpl
{
	bool GetExecutablePath(LPTSTR path, size_t len);

	CString mBrowserName;    // ブラウザ種類を表す文字列
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
	const CString& name,
	const CString& url
) : 
	AdhocCommandBase(name, name),
	in(std::make_unique<PImpl>())
{
	in->mBrowserName = browserName;
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

	CString str;
	str.Format(_T("%s %s"), (LPCTSTR)in->mBrowserName, (LPCTSTR)TEXT_BOOKMARK);

	return str;
}

BOOL URLCommand::Execute(Parameter* param)
{
	if (GetModifierKeyState(param, MASK_SHIFT) != 0) {
		// URLをクリップボードにコピー
		Clipboard::Copy(in->mUrl);
		return TRUE;
	}

	// URLをブラウザで開く
	Path path;
	if (in->GetExecutablePath(path, path.size()) == false) {
		return FALSE;
	}

	if (path.FileExists() == false) {
		CString msg(_T("Browser executable not found."));
		msg += _T("\n");
		msg += (LPCTSTR)path;
		AfxMessageBox(msg);
		return TRUE;
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	exec.Run((LPCTSTR)path, in->mUrl, process);

	return TRUE;
}

HICON URLCommand::GetIcon()
{
	// ブラウザに応じたアイコンを取得
	Path path;
	in->GetExecutablePath(path, path.size());
	return IconLoader::Get()->LoadIconFromPath((LPCTSTR)path);
}

launcherapp::core::Command*
URLCommand::Clone()
{
	return new URLCommand(in->mBrowserName, this->mName, in->mUrl);
}

CString URLCommand::GetSourceName()
{
	return mName;
}

bool URLCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (__super::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_EXTRACANDIDATE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidate*)this;
		return true;
	}
	return false;
}

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

