#include "pch.h"
#include "framework.h"
#include "commands/pathfind/PathExecuteCommand.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/common/SubProcess.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/LocalPathResolver.h"
#include "AppPreference.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using LocalPathResolver = soyokaze::utility::LocalPathResolver;
using ExecuteHistory = soyokaze::commands::common::ExecuteHistory;
using SubProcess = soyokaze::commands::common::SubProcess;

namespace soyokaze {
namespace commands {
namespace pathfind {

static CString EXE_EXT = _T(".exe");

using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

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
	CString mWord;
	CString mFullPath;
	bool mIsURL;
	bool mIsFromHistory;
	bool mIsExe;
};


PathExecuteCommand::PathExecuteCommand() : in(std::make_unique<PImpl>())
{
	in->mIsURL = false;
	in->mIsFromHistory = false;
	in->mIsExe = false;
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

BOOL PathExecuteCommand::Execute(const Parameter& param)
{
	if (in->mIsURL == false && PathFileExists(in->mFullPath) == FALSE) {
		return FALSE;
	}

	// 履歴に追加
	ExecuteHistory::GetInstance()->Add(_T("pathfind"), in->mWord, in->mFullPath);

	SubProcess exec(param);
	SubProcess::ProcessPtr process;
	if (exec.Run(in->mFullPath, param.GetParameterString(), process) == FALSE) {
		//in->mErrMsg = (LPCTSTR)process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

HICON PathExecuteCommand::GetIcon()
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
		this->mDescription = resolvedPath;

		in->mWord = word;
		in->mFullPath = resolvedPath;
		in->mIsFromHistory = false;
		return Pattern::WholeMatch;
	}

	return Pattern::Mismatch;
}

soyokaze::core::Command*
PathExecuteCommand::Clone()
{
	auto clonedObj = std::make_unique<PathExecuteCommand>();

	clonedObj->mDescription = this->mDescription;

	clonedObj->in->mResolver = in->mResolver;
	clonedObj->in->mFullPath = in->mFullPath;

	return clonedObj.release();
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze

