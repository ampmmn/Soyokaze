#include "pch.h"
#include "UserDirCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/AppProfile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

CString UserDirCommand::TYPE(_T("Builtin-UserDir"));

CString UserDirCommand::GetType()
{
	return TYPE;
}

UserDirCommand::UserDirCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("userdir"))
{
	mDescription = _T("【ユーザーフォルダ】");
}

UserDirCommand::~UserDirCommand()
{
}

BOOL UserDirCommand::Execute(const Parameter& param)
{
	TCHAR userDirPath[65536];
	CAppProfile::GetDirPath(userDirPath, 65536);
	_tcscat_s(userDirPath, _T("\\"));

	ShellExecCommand cmd;
	cmd.SetPath(userDirPath);

	Parameter paramEmpty;
	return cmd.Execute(paramEmpty);
}

HICON UserDirCommand::GetIcon()
{
	return IconLoader::Get()->LoadUserDirIcon();
}

soyokaze::core::Command* UserDirCommand::Clone()
{
	return new UserDirCommand();
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

