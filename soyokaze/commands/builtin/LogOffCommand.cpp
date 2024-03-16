#include "pch.h"
#include "framework.h"
#include "commands/builtin/LogOffCommand.h"
#include "core/CommandRepository.h"
#include "CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString LogOffCommand::TYPE(_T("Builtin-LogOff"));

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

soyokaze::core::Command* LogOffCommand::Clone()
{
	return new LogOffCommand();
}

}
}
}
