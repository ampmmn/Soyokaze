#include "pch.h"
#include "OpenInChromeAction.h"
#include "commands/common/SubProcess.h"
#include "utility/Path.h"

namespace launcherapp { namespace actions { namespace web {


using namespace launcherapp::commands::common;

static bool GetChromeExecutablePath(LPTSTR path, size_t len)
{
	UNREFERENCED_PARAMETER(len);

	size_t reqLen = 0;
	_tgetenv_s(&reqLen, path, MAX_PATH_NTFS, _T("PROGRAMFILES"));
	PathAppend(path, _T("Google\\Chrome\\Application\\chrome.exe"));

	return true;
}

OpenInChromeAction::OpenInChromeAction(const CString& url) : mURL(url)
{
}

OpenInChromeAction::~OpenInChromeAction()
{
}

// Action
// アクションの内容を示す名称
CString OpenInChromeAction::GetDisplayName()
{
	return _T("ChromeでURLを開く");
}

// アクションを実行する
bool OpenInChromeAction::Perform(Parameter* param, String* errMsg)
{
	// chrome.exeのパスを取得(初回のみ)
	static Path chromePath;
	if (chromePath.IsEmptyPath()) {
		GetChromeExecutablePath(chromePath, MAX_PATH_NTFS);
	}

	// chromeが見つからない
	if (chromePath.FileExists() == false) {
		if (errMsg) {
			*errMsg = "Chrome executable not found.";
			return false;
		}
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	exec.Run((LPCTSTR)chromePath, mURL, process);

	return true;
}

}}}

