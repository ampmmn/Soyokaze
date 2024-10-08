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

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(LockScreenCommand)

CString LockScreenCommand::GetType()
{
	return TYPE;
}

LockScreenCommand::LockScreenCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("lockscreen"))
{
	mDescription = _T("【スクリーンロック】");
	mCanSetConfirm = false;
	mCanDisable = true;
}

LockScreenCommand::LockScreenCommand(const LockScreenCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

LockScreenCommand::~LockScreenCommand()
{
}

BOOL LockScreenCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	return LockWorkStation();
}

HICON LockScreenCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5405);
}


launcherapp::core::Command* LockScreenCommand::Clone()
{
	return new LockScreenCommand(*this);
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

