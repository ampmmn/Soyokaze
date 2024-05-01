#include "pch.h"
#include "framework.h"
#include "commands/builtin/StandbyCommand.h"
#include "commands/builtin/SuspendCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <powrprof.h>
#pragma comment(lib, "powrprof.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString StandbyCommand::TYPE(_T("Builtin-Standby"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(StandbyCommand)

CString StandbyCommand::GetType()
{
	return TYPE;
}

StandbyCommand::StandbyCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("standby"))
{
	mDescription = _T("【PCをスタンバイ状態にする】");
	mCanSetConfirm = true;
	mCanDisable = true;
}

StandbyCommand::StandbyCommand(const StandbyCommand& rhs) :
	BuiltinCommandBase(rhs)
{
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
	if (mIsConfirmBeforeRun) {
		if (AfxMessageBox(_T("スタンバイ状態にしますか?"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) {
			return TRUE;
		}
	}
	return SuspendCommand::DoSuspend(FALSE);
}

launcherapp::core::Command* StandbyCommand::Clone()
{
	return new StandbyCommand(*this);
}

}
}
}
