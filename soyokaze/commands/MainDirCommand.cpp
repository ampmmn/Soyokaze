#include "pch.h"
#include "framework.h"
#include "commands/MainDirCommand.h"
#include "commands/ShellExecCommand.h"
#include "utility/AppProfile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

MainDirCommand::MainDirCommand() : mRefCount(1)
{
}

MainDirCommand::~MainDirCommand()
{
}

CString MainDirCommand::GetName()
{
	return _T("maindir");
}

CString MainDirCommand::GetDescription()
{
	return _T("【メインフォルダ】");
}

BOOL MainDirCommand::Execute()
{
	TCHAR mainDirPath[65536];
	GetModuleFileName(NULL, mainDirPath, 65536);
	PathRemoveFileSpec(mainDirPath);
	_tcscat_s(mainDirPath, _T("\\"));

	ShellExecCommand cmd;
	cmd.SetPath(mainDirPath);
	return cmd.Execute();
}

BOOL MainDirCommand::Execute(const Parameter& param)
{
	// 引数指定しても動作はかわらない
	return Execute();
}

CString MainDirCommand::GetErrorString()
{
	return _T("");
}

HICON MainDirCommand::GetIcon()
{
	return IconLoader::Get()->LoadMainDirIcon();
}

int MainDirCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

soyokaze::core::Command* MainDirCommand::Clone()
{
	return new MainDirCommand();
}

uint32_t MainDirCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t MainDirCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}
