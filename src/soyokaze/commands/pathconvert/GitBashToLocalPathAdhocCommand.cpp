#include "pch.h"
#include "framework.h"
#include "commands/pathconvert/GitBashToLocalPathAdhocCommand.h"
#include "commands/pathconvert/Icon.h"
#include "commands/common/SubProcess.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "commands/common/CommandParameterFunctions.h"
#include "utility/Path.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace pathconvert {

struct GitBashToLocalPathAdhocCommand::PImpl
{
	CString mFullPath;
	Icon mIcon;
	bool mIsExe{false};
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(GitBashToLocalPathAdhocCommand)

GitBashToLocalPathAdhocCommand::GitBashToLocalPathAdhocCommand() : in(std::make_unique<PImpl>())
{
}

GitBashToLocalPathAdhocCommand::~GitBashToLocalPathAdhocCommand()
{
}


CString GitBashToLocalPathAdhocCommand::GetName()
{
	return in->mFullPath;
}

CString GitBashToLocalPathAdhocCommand::GetGuideString()
{
	return _T("⏎:パスをコピー C-⏎:フォルダを開く S-⏎:開く");
}

CString GitBashToLocalPathAdhocCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool GitBashToLocalPathAdhocCommand::ShouldCopy(Parameter* param)
{
	// 何も修飾キーがおされてないならコピー操作をする
	return GetModifierKeyState(param, MASK_ALL) == 0;
}

BOOL GitBashToLocalPathAdhocCommand::Execute(Parameter* param)
{
	if (ShouldCopy(param)) {
		// クリップボードにコピー
		Clipboard::Copy(in->mFullPath);
		return TRUE;
	}
	else {
		// フォルダを開く or 開く
		if (Path::FileExists(in->mFullPath) == FALSE) {
			launcherapp::commands::common::PopupMessage(_T("パスは存在しません"));
			return TRUE;
		}

		SubProcess exec(param);
		SubProcess::ProcessPtr process;
		if (exec.Run(in->mFullPath, param->GetParameterString(), process) == FALSE) {
			this->mErrMsg = (LPCTSTR)process->GetErrorMessage();
			return FALSE;
		}
	}
	return TRUE;
}

HICON GitBashToLocalPathAdhocCommand::GetIcon()
{
	if (Path::FileExists(in->mFullPath) == FALSE) {
		// dummy
		return IconLoader::Get()->LoadUnknownIcon();
	}

	return in->mIcon.Load(in->mFullPath);
}

int GitBashToLocalPathAdhocCommand::Match(Pattern* pattern)
{
	CString wholeWord = pattern->GetWholeString();

	// /driveletter/...に合致するか
	if (IsGitBashPath(wholeWord)) {

		static tregex patReplace(_T("^ */([a-zA-Z])/(.*)$"));
		auto driveLetter = std::regex_replace(tstring(wholeWord), patReplace, _T("$1"));
		auto path = std::regex_replace(tstring(wholeWord), patReplace, _T("$2"));

		in->mFullPath.Format(_T("%s:\\%s"), driveLetter.c_str(), path.c_str());
		in->mFullPath.Replace(_T('/'), _T('\\'));

		return Pattern::WholeMatch;
	}
	return Pattern::Mismatch;
}

launcherapp::core::Command*
GitBashToLocalPathAdhocCommand::Clone()
{
	auto clonedObj = make_refptr<GitBashToLocalPathAdhocCommand>();

	clonedObj->mDescription = this->mDescription;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

bool GitBashToLocalPathAdhocCommand::IsGitBashPath(const CString& path)
{
	static tregex pat(_T("^ */[a-zA-Z]/.*$"));
	return std::regex_match(tstring(path), pat);
}

CString GitBashToLocalPathAdhocCommand::TypeDisplayName()
{
	return _T("git-bash To Local Path");
}


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


