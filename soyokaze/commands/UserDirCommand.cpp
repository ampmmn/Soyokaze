#include "pch.h"
#include "framework.h"
#include "commands/UserDirCommand.h"
#include "commands/ShellExecCommand.h"
#include "utility/AppProfile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UserDirCommand::UserDirCommand() :
	mRefCount(1)
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

BOOL UserDirCommand::Execute(const Parameter& param)
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

int UserDirCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

soyokaze::core::Command* UserDirCommand::Clone()
{
	return new UserDirCommand();
}

uint32_t UserDirCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t UserDirCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}
