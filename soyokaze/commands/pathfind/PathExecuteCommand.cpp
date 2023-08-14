#include "pch.h"
#include "framework.h"
#include "commands/pathfind/PathExecuteCommand.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/LocalPathResolver.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using LocalPathResolver = soyokaze::utility::LocalPathResolver;
using ExecuteHistory = soyokaze::commands::common::ExecuteHistory;

namespace soyokaze {
namespace commands {
namespace pathfind {


using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

static const tregex& GetURLRegex()
{
	static tregex reg(_T("https?://.+"));
	return reg;
}

struct PathExecuteCommand::PImpl
{
	LocalPathResolver mResolver;
	CString mWord;
	CString mFullPath;
	CString mExeExtension;
	bool mIsURL;
	bool mIsFromHistory;
};


PathExecuteCommand::PathExecuteCommand() : in(std::make_unique<PImpl>())
{
	in->mExeExtension = _T(".exe");
	in->mIsURL = false;
	in->mIsFromHistory = false;
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

CString PathExecuteCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE_ADHOC((LPCTSTR)IDS_COMMAND_PATHEXEC);
	static CString TEXT_TYPE_HISTORY((LPCTSTR)IDS_COMMAND_PATHEXEC_HISTORY);

	return in->mIsFromHistory ? TEXT_TYPE_HISTORY : TEXT_TYPE_ADHOC;
}

BOOL PathExecuteCommand::Execute(const Parameter& param)
{
	std::vector<CString> args;
	param.GetParameters(args);

	if (in->mIsURL == false && PathFileExists(in->mFullPath) == FALSE) {
		return FALSE;
	}

	ShellExecCommand::ATTRIBUTE attr;
	attr.mPath = in->mFullPath;

	for (int i = 0; i < args.size(); ++i) {
		if (i != 0) {
			attr.mParam += _T(" ");
		}
		attr.mParam += _T("\"");
		attr.mParam += args[i];
		attr.mParam += _T("\"");
	}

	// 履歴に追加
	ExecuteHistory::GetInstance()->Add(_T("pathfind"), in->mWord, in->mFullPath);

	ShellExecCommand cmd;
	cmd.SetAttribute(attr);
	return cmd.Execute(param);
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

	if (PathIsRelative(wholeWord) == FALSE && PathFileExists(wholeWord)) {
		this->mDescription = wholeWord;

		in->mWord = wholeWord;
		in->mFullPath = wholeWord;
		in->mIsFromHistory = false;
		return Pattern::WholeMatch;
	}

	CString word = pattern->GetFirstWord();
	if (in->mExeExtension.CompareNoCase(PathFindExtension(word)) != 0) {
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
	clonedObj->in->mExeExtension = in->mExeExtension;

	return clonedObj.release();
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze

