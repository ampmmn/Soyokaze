#include "pch.h"
#include "framework.h"
#include "LockScreenCommand.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString LockScreenCommand::TYPE(_T("Builtin-LockScreen"));

CString LockScreenCommand::GetType()
{
	return TYPE;
}

LockScreenCommand::LockScreenCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("lockscreen"))
{
	mDescription = _T("【スクリーンロック】");
}

LockScreenCommand::~LockScreenCommand()
{
}

BOOL LockScreenCommand::Execute(const Parameter& param)
{
	return LockWorkStation();
}

HICON LockScreenCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5405);
}


launcherapp::core::Command* LockScreenCommand::Clone()
{
	return new LockScreenCommand();
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

