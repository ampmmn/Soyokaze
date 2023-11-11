#include "pch.h"
#include "framework.h"
#include "ManagerCommand.h"
#include "core/CommandRepository.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString ManagerCommand::TYPE(_T("Builtin-Manager"));

CString ManagerCommand::GetType()
{
	return TYPE;
}

ManagerCommand::ManagerCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("manager"))
{
	mDescription = _T("【キーワードマネージャ】");
}

ManagerCommand::~ManagerCommand()
{
}

BOOL ManagerCommand::Execute(const Parameter& param)
{
	soyokaze::core::CommandRepository::GetInstance()->ManagerDialog();
	return TRUE;
}

HICON ManagerCommand::GetIcon()
{
	return IconLoader::Get()->LoadDefaultIcon();
}

soyokaze::core::Command* ManagerCommand::Clone()
{
	return new ManagerCommand();
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

