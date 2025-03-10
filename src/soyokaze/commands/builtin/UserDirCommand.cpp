#include "pch.h"
#include "UserDirCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/AppProfile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;

CString UserDirCommand::TYPE(_T("Builtin-UserDir"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(UserDirCommand)

CString UserDirCommand::GetType()
{
	return TYPE;
}

UserDirCommand::UserDirCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("userdir"))
{
	mDescription = _T("【ユーザーフォルダ】");
}

UserDirCommand::UserDirCommand(const UserDirCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

UserDirCommand::~UserDirCommand()
{
}

BOOL UserDirCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	std::vector<TCHAR> userDirPath(65536);
	CAppProfile::GetDirPath(userDirPath.data(), userDirPath.size(), false);
	_tcscat_s(userDirPath.data(), userDirPath.size(), _T("\\"));

	ShellExecCommand cmd;
	cmd.SetPath(userDirPath.data());

	return cmd.Execute(CommandParameterBuilder::EmptyParam());
}

HICON UserDirCommand::GetIcon()
{
	return IconLoader::Get()->LoadUserDirIcon();
}

launcherapp::core::Command* UserDirCommand::Clone()
{
	return new UserDirCommand(*this);
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

