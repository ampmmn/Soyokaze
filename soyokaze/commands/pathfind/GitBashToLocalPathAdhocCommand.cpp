#include "pch.h"
#include "framework.h"
#include "commands/pathfind/GitBashToLocalPathAdhocCommand.h"
#include "commands/common/SubProcess.h"
#include "commands/common/Clipboard.h"
#include "commands/common/Message.h"
#include "AppPreference.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using SubProcess = soyokaze::commands::common::SubProcess;
using Clipboard = soyokaze::commands::common::Clipboard;


namespace soyokaze {
namespace commands {
namespace pathfind {

struct GitBashToLocalPathAdhocCommand::PImpl
{
	bool IsEnable();

	CString mFullPath;
	bool mIsExe;
};


bool GitBashToLocalPathAdhocCommand::PImpl::IsEnable()
{
	auto pref = AppPreference::Get();
	return pref->IsEnableGitBashPath();
}

GitBashToLocalPathAdhocCommand::GitBashToLocalPathAdhocCommand() : in(std::make_unique<PImpl>())
{
	in->mIsExe = false;
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
	return _T("Enter:パスをコピー Ctrl-Enter:フォルダを開く Shift-Enter:開く");
}

CString GitBashToLocalPathAdhocCommand::GetTypeDisplayName()
{
	return _T("git-bash To Local Path");
}

bool GitBashToLocalPathAdhocCommand::ShouldCopy(const Parameter& param)
{
	// 何も修飾キーがおされてないならコピー操作をする
	return param.GetNamedParamBool(_T("CtrlKeyPressed")) == false &&
	       param.GetNamedParamBool(_T("ShiftKeyPressed")) == false &&
	       param.GetNamedParamBool(_T("AltKeyPressed")) == false &&
	       param.GetNamedParamBool(_T("WinKeyPressed")) == false;
}

BOOL GitBashToLocalPathAdhocCommand::Execute(const Parameter& param)
{
	if (ShouldCopy(param)) {
		// クリップボードにコピー
		Clipboard::Copy(in->mFullPath);
		return TRUE;
	}
	else {
		// フォルダを開く or 開く
		if (PathFileExists(in->mFullPath) == FALSE) {
			soyokaze::commands::common::PopupMessage(_T("パスは存在しません"));
			return TRUE;
		}

		SubProcess exec(param);
		SubProcess::ProcessPtr process;
		if (exec.Run(in->mFullPath, param.GetParameterString(), process) == FALSE) {
			this->mErrMsg = (LPCTSTR)process->GetErrorMessage();
			return FALSE;
		}
	}
	return TRUE;
}

HICON GitBashToLocalPathAdhocCommand::GetIcon()
{
	if (PathFileExists(in->mFullPath) == FALSE) {
		// dummy
		return IconLoader::Get()->LoadUnknownIcon();
	}

	SHFILEINFO sfi = {};
	HIMAGELIST hImgList =
		(HIMAGELIST)::SHGetFileInfo(in->mFullPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON);
	HICON hIcon = sfi.hIcon;
	return hIcon;
}

int GitBashToLocalPathAdhocCommand::Match(Pattern* pattern)
{
	if (in->IsEnable() == false) {
		return Pattern::Mismatch;
	}

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

soyokaze::core::Command*
GitBashToLocalPathAdhocCommand::Clone()
{
	auto clonedObj = std::make_unique<GitBashToLocalPathAdhocCommand>();

	clonedObj->mDescription = this->mDescription;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}

bool GitBashToLocalPathAdhocCommand::IsGitBashPath(const CString& path)
{
	static tregex pat(_T("^ */[a-zA-Z]/.*$"));
	return std::regex_match(tstring(path), pat);
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze


