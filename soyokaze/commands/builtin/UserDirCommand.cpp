#include "pch.h"
#include "framework.h"
#include "commands/builtin/UserDirCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/AppProfile.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

CString UserDirCommand::GetType() { return _T("Builtin-UserDir"); }

UserDirCommand::UserDirCommand(LPCTSTR name) :
	mRefCount(1)
{
	mName = name ? name : _T("userdir");
}

UserDirCommand::~UserDirCommand()
{
}

CString UserDirCommand::GetName()
{
	return mName;
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

bool UserDirCommand::IsEditable()
{
	return false;
}


int UserDirCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* UserDirCommand::Clone()
{
	return new UserDirCommand();
}

bool UserDirCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
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

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

