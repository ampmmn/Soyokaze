#include "pch.h"
#include "framework.h"
#include "ReloadCommand.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString ReloadCommand::TYPE(_T("Builtin-Reload"));

CString ReloadCommand::GetType()
{
	return TYPE;
}

ReloadCommand::ReloadCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("reload"))
{
	mDescription = _T("【設定のリロード】");
}

ReloadCommand::~ReloadCommand()
{
}

BOOL ReloadCommand::Execute(const Parameter& param)
{
	return soyokaze::core::CommandRepository::GetInstance()->Load();
}

HICON ReloadCommand::GetIcon()
{
	return IconLoader::Get()->LoadReloadIcon();
}


soyokaze::core::Command* ReloadCommand::Clone()
{
	return new ReloadCommand();
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

