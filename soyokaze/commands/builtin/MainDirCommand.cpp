#include "pch.h"
#include "framework.h"
#include "MainDirCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "icon/IconLoader.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {


using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;


CString MainDirCommand::TYPE(_T("Builtin-MainDir"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(MainDirCommand)

CString MainDirCommand::GetType()
{
	return TYPE;
}

MainDirCommand::MainDirCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("maindir"))
{
	mDescription = _T("【メインフォルダ】");
}

MainDirCommand::MainDirCommand(const MainDirCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

MainDirCommand::~MainDirCommand()
{
}

BOOL MainDirCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	Path mainDirPath(Path::MODULEFILEDIR);
	_tcscat_s(mainDirPath, mainDirPath.size(), _T("\\"));

	ShellExecCommand cmd;
	cmd.SetPath(mainDirPath);

	return cmd.Execute(CommandParameterBuilder::EmptyParam());
}

HICON MainDirCommand::GetIcon()
{
	return IconLoader::Get()->LoadMainDirIcon();
}

launcherapp::core::Command* MainDirCommand::Clone()
{
	return new MainDirCommand(*this);
}

}
}
}
