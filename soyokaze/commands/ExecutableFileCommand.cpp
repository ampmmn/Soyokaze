#include "pch.h"
#include "framework.h"
#include "commands/ExecutableFileCommand.h"
#include "commands/ShellExecCommand.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct ExecutableFileCommand::PImpl
{
	std::vector<CString> targetDirs;
	CString mFullPath;
	CString mDescription;
	CString mExeExtension;
	uint32_t mRefCount;
};


ExecutableFileCommand::ExecutableFileCommand() : in(new PImpl)
{
	in->mRefCount = 1;
	in->mExeExtension = _T(".exe");

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

ExecutableFileCommand::~ExecutableFileCommand()
{
	delete in;
}

CString ExecutableFileCommand::GetName()
{
	if (in->mFullPath.IsEmpty()) {
		return _T("");
	}
	return PathFindFileName(in->mFullPath);
}

CString ExecutableFileCommand::GetDescription()
{
	return in->mDescription;
}

BOOL ExecutableFileCommand::Execute()
{
	if (PathFileExists(in->mFullPath) == FALSE) {
		return FALSE;
	}

	ShellExecCommand cmd;
	cmd.SetPath(in->mFullPath);
	return cmd.Execute();
}

BOOL ExecutableFileCommand::Execute(const Parameter& param)
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

	ShellExecCommand cmd;
	cmd.SetAttribute(attr);
	return cmd.Execute();

	// ぱらめーたしていはさぽーとしない
	return Execute();
}

CString ExecutableFileCommand::GetErrorString()
{
	return _T("");
}

HICON ExecutableFileCommand::GetIcon()
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

int ExecutableFileCommand::Match(Pattern* pattern)
{
	CString word = pattern->GetOriginalPattern();

	if (PathIsRelative(word) == FALSE && PathFileExists(word)) {
		in->mFullPath = word;
		in->mDescription = word;
		return Pattern::WholeMatch;
	}

	if (in->mExeExtension.CompareNoCase(PathFindExtension(word)) != 0) {
		word += _T(".exe");
	}

	// targetDirsにがいとうするexeがないかをさがす
	TCHAR path[MAX_PATH_NTFS];
	for (const auto& dir : in->targetDirs) {
		_tcscpy_s(path, dir);
		PathAppend(path, word);

		if (PathFileExists(path)) {
			in->mFullPath = path;
			in->mDescription = path;
			return Pattern::WholeMatch;
		}
	}
	return Pattern::Mismatch;
}

bool ExecutableFileCommand::IsEditable()
{
	return false;
}

int ExecutableFileCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
ExecutableFileCommand::Clone()
{
	auto clonedObj = new ExecutableFileCommand();

	clonedObj->in->targetDirs = in->targetDirs;
	clonedObj->in->mFullPath = in->mFullPath;
	clonedObj->in->mDescription = in->mDescription;
	clonedObj->in->mExeExtension = in->mExeExtension;

	return clonedObj;
}

bool ExecutableFileCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t ExecutableFileCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t ExecutableFileCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}
