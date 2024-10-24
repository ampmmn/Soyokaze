#include "pch.h"
#include "framework.h"
#include "PathExecuteCommand.h"
#include "commands/pathfind/ExcludePathList.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/common/SubProcess.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/LocalPathResolver.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using LocalPathResolver = launcherapp::utility::LocalPathResolver;
using ExecuteHistory = launcherapp::commands::common::ExecuteHistory;
using SubProcess = launcherapp::commands::common::SubProcess;

namespace launcherapp {
namespace commands {
namespace pathfind {

static CString EXE_EXT = _T(".exe");

using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;

static const tregex& GetURLRegex()
{
	static tregex reg(_T("https?://.+"));
	return reg;
}

struct PathExecuteCommand::PImpl
{
	void Reload()
	{
		mResolver.ResetPath();

		// 追加パスを登録
		auto pref = AppPreference::Get();
		std::vector<CString> paths;
		pref->GetAdditionalPaths(paths);

		for (auto& path : paths) {
			mResolver.AddPath(path);
		}
	}

	LocalPathResolver mResolver;
	ExcludePathList* mExcludeFiles = nullptr;
	CString mWord;
	CString mFullPath;
	bool mIsURL = false;
	bool mIsFromHistory = false;
	bool mIsExe = false;
};


PathExecuteCommand::PathExecuteCommand(
	ExcludePathList* excludeList
) : in(std::make_unique<PImpl>())
{
	in->mExcludeFiles = excludeList;
}

PathExecuteCommand::~PathExecuteCommand()
{
}

void PathExecuteCommand::SetFullPath(const CString& path, bool isFromHistory)
{
	this->mDescription = path;

	in->mFullPath = path;

	const tregex& regURL = GetURLRegex();
	in->mIsURL = (std::regex_search((LPCTSTR)path, regURL));
	in->mIsFromHistory = isFromHistory;
	if (in->mIsURL == false) {
		in->mIsExe = EXE_EXT.CompareNoCase(PathFindExtension(path)) == 0;
	}
}

void PathExecuteCommand::Reload()
{
	in->Reload();
}

CString PathExecuteCommand::GetName()
{
	if (in->mFullPath.IsEmpty()) {
		return _T("");
	}

	if (in->mIsURL) {
		return in->mFullPath;
	}

	return PathFindFileName(in->mFullPath);
}

CString PathExecuteCommand::GetGuideString()
{
	if (in->mIsURL) {
		return _T("Enter:ブラウザで開く");
	}
	else if (in->mIsExe) {
		return _T("Enter:開く Ctrl-Enter:フォルダを開く");
	}
	else {
		return _T("Enter:開く Ctrl-Enter:フォルダを開く");
	}
}

CString PathExecuteCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE_ADHOC((LPCTSTR)IDS_COMMAND_PATHEXEC);
	static CString TEXT_TYPE_HISTORY((LPCTSTR)IDS_COMMAND_PATHEXEC_HISTORY);

	return in->mIsFromHistory ? TEXT_TYPE_HISTORY : TEXT_TYPE_ADHOC;
}

BOOL PathExecuteCommand::Execute(Parameter* param)
{
	if (in->mIsURL == false && PathFileExists(in->mFullPath) == FALSE) {
		return FALSE;
	}

	// 履歴に追加
	ExecuteHistory::GetInstance()->Add(_T("pathfind"), in->mWord, in->mFullPath);

	SubProcess exec(param);
	SubProcess::ProcessPtr process;
	if (exec.Run(in->mFullPath, param->GetParameterString(), process) == FALSE) {
		//in->mErrMsg = (LPCTSTR)process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

HICON PathExecuteCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mFullPath);
}

int PathExecuteCommand::Match(Pattern* pattern)
{
	CString wholeWord = pattern->GetWholeString();

	// URLパターンマッチするかを判定
	const tregex& regURL = GetURLRegex();
	if (std::regex_search((LPCTSTR)wholeWord, regURL)) {
		this->mDescription = wholeWord;

		in->mWord = wholeWord;
		in->mFullPath = wholeWord;
		in->mIsURL = true;
		in->mIsFromHistory = false;
		return Pattern::WholeMatch;
	}

	in->mIsURL = false;

	// 絶対パス指定、かつ、存在するパスの場合は候補として表示
	if (PathIsRelative(wholeWord) == FALSE && PathFileExists(wholeWord)) {
		this->mDescription = wholeWord;

		in->mWord = wholeWord;
		in->mFullPath = wholeWord;
		in->mIsFromHistory = false;
		return Pattern::WholeMatch;
	}

	CString word = pattern->GetFirstWord();
	if (EXE_EXT.CompareNoCase(PathFindExtension(word)) != 0) {
		word += _T(".exe");
	}

	if (PathIsRelative(word) == FALSE) {
		return Pattern::Mismatch;
	}

	// 相対パスを解決する
	CString resolvedPath;
	if (in->mResolver.Resolve(word, resolvedPath)) {

		// 除外対象に含まれなければ
		if (in->mExcludeFiles &&
		    in->mExcludeFiles->Contains(resolvedPath) == false) {
			this->mDescription = resolvedPath;

			in->mWord = word;
			in->mFullPath = resolvedPath;
			in->mIsFromHistory = false;
			return Pattern::WholeMatch;
		}
	}

	return Pattern::Mismatch;
}

launcherapp::core::Command*
PathExecuteCommand::Clone()
{
	auto clonedObj = make_refptr<PathExecuteCommand>();

	clonedObj->mDescription = this->mDescription;

	clonedObj->in->mResolver = in->mResolver;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

