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
	CString mDescription;
	CString mExeExtension;
	bool mIsURL;
	bool mIsFromHistory;
	uint32_t mRefCount;
};


PathExecuteCommand::PathExecuteCommand() : in(new PImpl)
{
	in->mRefCount = 1;
	in->mExeExtension = _T(".exe");
	in->mIsURL = false;
	in->mIsFromHistory = false;
}

PathExecuteCommand::~PathExecuteCommand()
{
	delete in;
}

void PathExecuteCommand::SetFullPath(const CString& path, bool isFromHistory)
{
	in->mFullPath = path;
	in->mDescription = path;

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

CString PathExecuteCommand::GetDescription()
{
	return in->mDescription;
}

CString PathExecuteCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE_ADHOC((LPCTSTR)IDS_COMMAND_PATHEXEC);
	static CString TEXT_TYPE_HISTORY((LPCTSTR)IDS_COMMAND_PATHEXEC_HISTORY);

	return in->mIsFromHistory ? TEXT_TYPE_HISTORY : TEXT_TYPE_ADHOC;
}

BOOL PathExecuteCommand::Execute()
{
	if (in->mIsURL == false && PathFileExists(in->mFullPath) == FALSE) {
		return FALSE;
	}

	// 履歴に追加
	ExecuteHistory::GetInstance()->Add(_T("pathfind"), in->mWord, in->mFullPath);

	ShellExecCommand cmd;
	cmd.SetPath(in->mFullPath);
	return cmd.Execute();
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
	return cmd.Execute();
}

CString PathExecuteCommand::GetErrorString()
{
	return _T("");
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
	CString word = pattern->GetOriginalPattern();

	// URLパターンマッチするかを判定
	const tregex& regURL = GetURLRegex();
	if (std::regex_search((LPCTSTR)word, regURL)) {
		in->mWord = word;
		in->mFullPath = word;
		in->mDescription = word;
		in->mIsURL = true;
		in->mIsFromHistory = false;
		return Pattern::WholeMatch;
	}

	in->mIsURL = false;

	if (PathIsRelative(word) == FALSE && PathFileExists(word)) {
		in->mWord = word;
		in->mFullPath = word;
		in->mDescription = word;
		in->mIsFromHistory = false;
		return Pattern::WholeMatch;
	}

	if (in->mExeExtension.CompareNoCase(PathFindExtension(word)) != 0) {
		word += _T(".exe");
	}

	if (PathIsRelative(word) == FALSE) {
		return Pattern::Mismatch;
	}

	// 相対パスを解決する
	CString resolvedPath;
	if (in->mResolver.Resolve(word, resolvedPath)) {
		in->mWord = word;
		in->mFullPath = resolvedPath;
		in->mDescription = resolvedPath;
		in->mIsFromHistory = false;
		return Pattern::WholeMatch;
	}

	return Pattern::Mismatch;
}

bool PathExecuteCommand::IsEditable()
{
	return false;
}

int PathExecuteCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
PathExecuteCommand::Clone()
{
	auto clonedObj = new PathExecuteCommand();

	clonedObj->in->mResolver = in->mResolver;
	clonedObj->in->mFullPath = in->mFullPath;
	clonedObj->in->mDescription = in->mDescription;
	clonedObj->in->mExeExtension = in->mExeExtension;

	return clonedObj;
}

bool PathExecuteCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t PathExecuteCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t PathExecuteCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze

