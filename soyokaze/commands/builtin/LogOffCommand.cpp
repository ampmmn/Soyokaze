#include "pch.h"
#include "framework.h"
#include "commands/builtin/LogOffCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString LogOffCommand::TYPE(_T("Builtin-LogOff"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(LogOffCommand)

CString LogOffCommand::GetType()
{
	return TYPE;
}

LogOffCommand::LogOffCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("logoff"))
{
	mDescription = _T("【ログオフする】");
}

LogOffCommand::~LogOffCommand()
{
}

HICON LogOffCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-325);
}


BOOL LogOffCommand::Execute(const Parameter& param)
{
	ExitWindows(0, 0);
	return TRUE;
}

launcherapp::core::Command* LogOffCommand::Clone()
{
	return new LogOffCommand();
}

launcherapp::core::Command* LogOffCommand::Create(LPCTSTR name)
{
	return new LogOffCommand(name);
}

}
}
}
