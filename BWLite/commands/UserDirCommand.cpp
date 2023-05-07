#include "pch.h"
#include "framework.h"
#include "commands/UserDirCommand.h"
#include "commands/ShellExecCommand.h"
#include "AppProfile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UserDirCommand::UserDirCommand()
{
}

UserDirCommand::~UserDirCommand()
{
}

CString UserDirCommand::GetName()
{
	return _T("userdir");
}

CString UserDirCommand::GetDescription()
{
	return _T("【ユーザーフォルダ】");
}

BOOL UserDirCommand::Execute()
{
	TCHAR userDirPath[65536];
	CAppProfile::GetDirPath(userDirPath, 65536);
	_tcscat_s(userDirPath, _T("\\"));

	ShellExecCommand cmd;
	cmd.SetPath(userDirPath);
	return cmd.Execute();
}

BOOL UserDirCommand::Execute(const std::vector<CString>& args)
{
	// 引数指定しても動作はかわらない
	return Execute();
}

CString UserDirCommand::GetErrorString()
{
	return _T("");
}

HICON UserDirCommand::GetIcon()
{
	return IconLoader::Get()->LoadUserDirIcon();
}

BOOL UserDirCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

Command* UserDirCommand::Clone()
{
	return new UserDirCommand();
}

