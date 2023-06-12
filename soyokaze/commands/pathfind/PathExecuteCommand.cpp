#include "pch.h"
#include "framework.h"
#include "commands/pathfind/PathExecuteCommand.h"
#include "commands/pathfind/ExecuteHistory.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace pathfind {


using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;


struct PathExecuteCommand::PImpl
{
	std::vector<CString> targetDirs;
	ExecuteHistory* mHistoryPtr;
	CString mWord;
	CString mFullPath;
	CString mDescription;
	CString mExeExtension;
	uint32_t mRefCount;
};


PathExecuteCommand::PathExecuteCommand() : in(new PImpl)
{
	in->mRefCount = 1;
	in->mExeExtension = _T(".exe");
	in->mHistoryPtr = nullptr;

	LPCTSTR PATH = _T("PATH");

	// かんきょうへんすうPATH
	size_t reqLen = 0;
	if (_tgetenv_s(&reqLen, NULL, 0, PATH) != 0 || reqLen == 0) {
		return;
	}
	
	CString val;
	TCHAR* p = val.GetBuffer((int)reqLen);
	_tgetenv_s(&reqLen, p, reqLen, PATH);
	val.ReleaseBuffer();


	int n = 0;
	CString item = val.Tokenize(_T(";"), n);
	while(item.IsEmpty() == FALSE) {

		if (PathIsDirectory(item)) {
			in->targetDirs.push_back(item);
		}

		item = val.Tokenize(_T(";"), n);
	}
}

PathExecuteCommand::~PathExecuteCommand()
{
	delete in;
}

void PathExecuteCommand::SetHistoryList(
	ExecuteHistory* historyPtr
)
{
	in->mHistoryPtr = historyPtr;
}

void PathExecuteCommand::SetFullPath(const CString& path)
{
	in->mFullPath = path;
	in->mDescription = path;
}


CString PathExecuteCommand::GetName()
{
	if (in->mFullPath.IsEmpty()) {
		return _T("");
	}
	return PathFindFileName(in->mFullPath);
}

CString PathExecuteCommand::GetDescription()
{
	return in->mDescription;
}

BOOL PathExecuteCommand::Execute()
{
	if (PathFileExists(in->mFullPath) == FALSE) {
		return FALSE;
	}

	if (in->mHistoryPtr) {
		in->mHistoryPtr->Add(in->mWord, in->mFullPath);
	}

	ShellExecCommand cmd;
	cmd.SetPath(in->mFullPath);
	return cmd.Execute();
}

BOOL PathExecuteCommand::Execute(const Parameter& param)
{
	std::vector<CString> args;
	param.GetParameters(args);

	if (PathFileExists(in->mFullPath) == FALSE) {
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

	if (in->mHistoryPtr) {
		in->mHistoryPtr->Add(in->mWord, in->mFullPath);
	}

	ShellExecCommand cmd;
	cmd.SetAttribute(attr);
	return cmd.Execute();

	// ぱらめーたしていはさぽーとしない
	return Execute();
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

	if (PathIsRelative(word) == FALSE && PathFileExists(word)) {
		in->mWord = word;
		in->mFullPath = word;
		in->mDescription = word;
		return Pattern::WholeMatch;
	}

	if (in->mExeExtension.CompareNoCase(PathFindExtension(word)) != 0) {
		word += _T(".exe");
	}

	if (PathIsRelative(word) == FALSE) {
		return Pattern::Mismatch;
	}

	// targetDirsにがいとうするexeがないかをさがす
	TCHAR path[MAX_PATH_NTFS];
	for (const auto& dir : in->targetDirs) {
		_tcscpy_s(path, dir);
		PathAppend(path, word);

		if (PathFileExists(path)) {
			in->mWord = word;
			in->mFullPath = path;
			in->mDescription = path;
			return Pattern::WholeMatch;
		}
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

	clonedObj->in->targetDirs = in->targetDirs;
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

