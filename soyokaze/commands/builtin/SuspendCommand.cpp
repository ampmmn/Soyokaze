#include "pch.h"
#include "framework.h"
#include "commands/builtin/SuspendCommand.h"
#include "core/CommandRepository.h"
#include "CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <powrprof.h>
#pragma comment(lib, "powrprof.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString SuspendCommand::TYPE(_T("Builtin-Suspend"));

CString SuspendCommand::GetType()
{
	return TYPE;
}

SuspendCommand::SuspendCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("suspend"))
{
	mDescription = _T("【PCをサスペンド状態にする】");
}

SuspendCommand::~SuspendCommand()
{
}

HICON SuspendCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-329);
}


BOOL SuspendCommand::Execute(const Parameter& param)
{
	return SuspendCommand::DoSuspend(TRUE);
}

soyokaze::core::Command* SuspendCommand::Clone()
{
	return new SuspendCommand();
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
		SetSuspendState(isSuspend, FALSE, FALSE);
		return TRUE;
	}
	else {
		return FALSE;
	}
}


}
}
}
