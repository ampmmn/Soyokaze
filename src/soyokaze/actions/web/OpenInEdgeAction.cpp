#include "pch.h"
#include "OpenInEdgeAction.h"
#include "commands/common/SubProcess.h"
#include "utility/Path.h"

namespace launcherapp { namespace actions { namespace web {


using namespace launcherapp::commands::common;

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


OpenInEdgeAction::OpenInEdgeAction(const CString& url) : mURL(url)
{
}

OpenInEdgeAction::~OpenInEdgeAction()
{
}

// Action
// アクションの内容を示す名称
CString OpenInEdgeAction::GetDisplayName()
{
	return _T("EdgeでURLを開く");
}

// アクションを実行する
bool OpenInEdgeAction::Perform(Parameter* param, String* errMsg)
{
	// chrome.exeのパスを取得(初回のみ)
	static Path edgePath;
	if (edgePath.IsEmptyPath()) {
		GetEdgeExecutablePath(edgePath, MAX_PATH_NTFS);
	}

	// edgeが見つからない
	if (edgePath.FileExists() == false) {
		if (errMsg) {
			*errMsg = "Edge executable not found.";
			return false;
		}
	}

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	exec.Run((LPCTSTR)edgePath, mURL, process);

	return true;
}

}}}

