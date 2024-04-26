#include "pch.h"
#include "framework.h"
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

CString ShutdownCommand::TYPE(_T("Builtin-Shutdown"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(ShutdownCommand)

CString ShutdownCommand::GetType()
{
	return TYPE;
}

ShutdownCommand::ShutdownCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("shutdown"))
{
	mDescription = _T("【PCをシャットダウンする】");
}

ShutdownCommand::~ShutdownCommand()
{
}

HICON ShutdownCommand::GetIcon()
{
	return IconLoader::Get()->GetShell32Icon(-329);
}


BOOL ShutdownCommand::Execute(const Parameter& param)
{
	return DoExit(EWX_SHUTDOWN | EWX_FORCEIFHUNG);
}

launcherapp::core::Command* ShutdownCommand::Clone()
{
	return new ShutdownCommand();
}

launcherapp::core::Command* ShutdownCommand::Create(LPCTSTR name)
{
	return new ShutdownCommand(name);
}

BOOL ShutdownCommand::DoExit(UINT uFlags)
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
		ExitWindowsEx(uFlags, 0);
		return TRUE;
	}
	else {
		return FALSE;
	}
}


}
}
}
