#include "pch.h"
#include "framework.h"
#include "commands/builtin/StandbyCommand.h"
#include "commands/builtin/SuspendCommand.h"
#include "core/CommandRepository.h"
#include "CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <powrprof.h>
#pragma comment(lib, "powrprof.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString StandbyCommand::TYPE(_T("Builtin-Standby"));

CString StandbyCommand::GetType()
{
	return TYPE;
}

StandbyCommand::StandbyCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("standby"))
{
	mDescription = _T("【PCをスタンバイ状態にする】");
}

StandbyCommand::~StandbyCommand()
{
}

HICON StandbyCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-329);
}


BOOL StandbyCommand::Execute(const Parameter& param)
{
	return SuspendCommand::DoSuspend(FALSE);
}

soyokaze::core::Command* StandbyCommand::Clone()
{
	return new StandbyCommand();
}

}
}
}
