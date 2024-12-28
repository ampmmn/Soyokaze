#include "pch.h"
#include "framework.h"
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

CString SuspendCommand::TYPE(_T("Builtin-Suspend"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(SuspendCommand)

CString SuspendCommand::GetType()
{
	return TYPE;
}

SuspendCommand::SuspendCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("suspend"))
{
	mDescription = _T("【PCをサスペンド状態にする】");
	mCanSetConfirm = true;
	mCanDisable = true;
}

SuspendCommand::SuspendCommand(const SuspendCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

SuspendCommand::~SuspendCommand()
{
}

HICON SuspendCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-329);
}


BOOL SuspendCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	if (mIsConfirmBeforeRun) {
		if (AfxMessageBox(_T("サスペンドしますか?"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) {
			return TRUE;
		}
	}
	return SuspendCommand::DoSuspend(TRUE);
}

launcherapp::core::Command* SuspendCommand::Clone()
{
	return new SuspendCommand(*this);
}

BOOL SuspendCommand::DoSuspend(BOOL isSuspend)
{
	// 特権を付与する
	struct Handle {
		Handle() : mHandle(nullptr) {}
		~Handle() { if (mHandle) { CloseHandle(mHandle); } }
		HANDLE mHandle;
	} token;

	BOOL isOK = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token.mHandle);
	if (!isOK) {
		return FALSE;
	}

	LUID luid;
	isOK = LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid);
	if (!isOK) {
		return FALSE;
	}
	TOKEN_PRIVILEGES priv;
	priv.PrivilegeCount = 1;
	priv.Privileges[0].Luid = luid;
	priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(token.mHandle, FALSE, &priv, 0, 0, 0);

	if (GetLastError() == ERROR_SUCCESS) {
		SetSuspendState((BOOLEAN)isSuspend, (BOOLEAN)FALSE, (BOOLEAN)FALSE);
		return TRUE;
	}
	else {
		return FALSE;
	}
}


}
}
}
