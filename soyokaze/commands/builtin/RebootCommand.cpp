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

namespace launcherapp {
namespace commands {
namespace builtin {

CString RebootCommand::TYPE(_T("Builtin-Reboot"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(RebootCommand)

CString RebootCommand::GetType()
{
	return TYPE;
}

RebootCommand::RebootCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("reboot"))
{
	mDescription = _T("【PCを再起動する】");
	mCanSetConfirm = true;
	mCanDisable = true;
}

RebootCommand::RebootCommand(const RebootCommand& rhs) :
	BuiltinCommandBase(rhs)
{
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
	UNREFERENCED_PARAMETER(param);

	if (mIsConfirmBeforeRun) {
		if (AfxMessageBox(_T("再起動しますか?"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) {
			return TRUE;
		}
	}
	return ShutdownCommand::DoExit(EWX_REBOOT | EWX_FORCEIFHUNG);
}

launcherapp::core::Command* RebootCommand::Clone()
{
	return new RebootCommand(*this);
}

}
}
}
