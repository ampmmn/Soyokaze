#include "pch.h"
#include "WebHistoryAdhocCommand.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/common/SubProcess.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "utility/Path.h"
#include "icon/IconLoader.h"
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

struct WebHistoryAdhocCommand::PImpl
{
	bool GetExecutablePath(LPTSTR path, size_t len);

	CString mName;
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

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(WebHistoryAdhocCommand)

WebHistoryAdhocCommand::WebHistoryAdhocCommand(
	const CString& name, 
	const HISTORY& item
) : 
	AdhocCommandBase(_T(""), item.mDisplayName),
	in(std::make_unique<PImpl>())
{
	in->mName = name;
	in->mHistory = item;
}

WebHistoryAdhocCommand::~WebHistoryAdhocCommand()
{
}

CString WebHistoryAdhocCommand::GetName()
{
	return in->mName + _T(" ") + in->mHistory.mDisplayName;
}

CString WebHistoryAdhocCommand::GetGuideString()
{
	return _T("⏎:ブラウザで開く S-⏎:URLをクリップボードにコピー");
}

CString WebHistoryAdhocCommand::GetTypeDisplayName()
{
	return WebHistoryAdhocCommand::TypeDisplayName((LPCTSTR)in->mHistory.mBrowserName);
}

BOOL WebHistoryAdhocCommand::Execute(Parameter* param)
{
	bool isShiftKeyPressed = GetModifierKeyState(param, MASK_SHIFT) != 0;
	if (isShiftKeyPressed) {
		// URLをクリップボードにコピー
		Clipboard::Copy(in->mHistory.mUrl);
		return TRUE;
	}

	// URLをブラウザで開く
	std::vector<TCHAR> path(MAX_PATH_NTFS);
	if (in->GetExecutablePath(path.data(), MAX_PATH_NTFS) == false) {
		return FALSE;
	}

	if (Path::FileExists(path.data()) == FALSE) {
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
	return new WebHistoryAdhocCommand(in->mName, in->mHistory);
}

CString WebHistoryAdhocCommand::GetSourceName()
{
	return in->mName;
}

bool WebHistoryAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_EXTRACANDIDATE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidate*)this;
		return true;
	}
	return false;
}

CString WebHistoryAdhocCommand::TypeDisplayName(LPCTSTR browserName)
{
	static CString TEXT_HISTORY((LPCTSTR)IDS_COMMAND_HISTORY);

	CString str;
	str.Format(_T("%s %s"), browserName, (LPCTSTR)TEXT_HISTORY);

	return str;
}


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

