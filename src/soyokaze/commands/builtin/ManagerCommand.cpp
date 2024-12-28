#include "pch.h"
#include "framework.h"
#include "ManagerCommand.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString ManagerCommand::TYPE(_T("Builtin-Manager"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(ManagerCommand)

CString ManagerCommand::GetType()
{
	return TYPE;
}

ManagerCommand::ManagerCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("manager"))
{
	mDescription = _T("【キーワードマネージャ】");
}

ManagerCommand::ManagerCommand(const ManagerCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

ManagerCommand::~ManagerCommand()
{
}

BOOL ManagerCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	launcherapp::core::CommandRepository::GetInstance()->ManagerDialog();
	return TRUE;
}

HICON ManagerCommand::GetIcon()
{
	return IconLoader::Get()->LoadKeywordManagerIcon();
}

launcherapp::core::Command* ManagerCommand::Clone()
{
	return new ManagerCommand(*this);
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

