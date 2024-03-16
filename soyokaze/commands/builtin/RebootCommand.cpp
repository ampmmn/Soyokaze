#include "pch.h"
#include "framework.h"
#include "commands/builtin/RebootCommand.h"
#include "commands/builtin/ShutdownCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString RebootCommand::TYPE(_T("Builtin-Reboot"));

CString RebootCommand::GetType()
{
	return TYPE;
}

RebootCommand::RebootCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("reboot"))
{
	mDescription = _T("【PCを再起動する】");
}

RebootCommand::~RebootCommand()
{
}

HICON RebootCommand::GetIcon()
{
	return IconLoader::Get()->LoadReloadIcon();
}


BOOL RebootCommand::Execute(const Parameter& param)
{
	return ShutdownCommand::DoExit(EWX_REBOOT | EWX_FORCEIFHUNG);
}

soyokaze::core::Command* RebootCommand::Clone()
{
	return new RebootCommand();
}

}
}
}
