#include "pch.h"
#include "framework.h"
#include "ReloadCommand.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString ReloadCommand::TYPE(_T("Builtin-Reload"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(ReloadCommand)

CString ReloadCommand::GetType()
{
	return TYPE;
}

ReloadCommand::ReloadCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("reload"))
{
	mDescription = _T("【設定のリロード】");
	mCanSetConfirm = false;
	mCanDisable = true;
}

ReloadCommand::ReloadCommand(const ReloadCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

ReloadCommand::~ReloadCommand()
{
}

BOOL ReloadCommand::Execute(const Parameter& param)
{
	UNREFERENCED_PARAMETER(param);

	return launcherapp::core::CommandRepository::GetInstance()->Load();
}

HICON ReloadCommand::GetIcon()
{
	return IconLoader::Get()->LoadReloadIcon();
}


launcherapp::core::Command* ReloadCommand::Clone()
{
	return new ReloadCommand(*this);
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

