#include "pch.h"
#include "framework.h"
#include "WinScpCommand.h"
#include "commands/common/SubProcess.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp { namespace commands { namespace winscp {

struct WinScpCommand::PImpl
{
	CommandParam* mParam{nullptr};
	CString mSessionName;
	CString mErrorMsg;
};




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(WinScpCommand)


WinScpCommand::WinScpCommand(CommandParam* param, const CString& sessionName) : in(std::make_unique<PImpl>())
{
	in->mParam = param;
	in->mSessionName = sessionName;
	this->mName = sessionName;
	this->mDescription = sessionName;
}

WinScpCommand::~WinScpCommand()
{
}

CString WinScpCommand::GetGuideString()
{
	return _T("⏎:セッションを開く");
}

CString WinScpCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL WinScpCommand::Execute(Parameter* param)
{
	CString exeFilePath;
	if (in->mParam->ResolveExecutablePath(exeFilePath) == false) {
		spdlog::error("Failed to get winscp executable path");
		return TRUE;
	}

	SubProcess exec(param);

	CString argStr;
	argStr.Format(_T("\"%s\""), (LPCTSTR)in->mSessionName);

	// プロセスを実行する
	SubProcess::ProcessPtr process;
	if (exec.Run(exeFilePath, argStr, process) == FALSE) {
		in->mErrorMsg = (LPCTSTR)process->GetErrorMessage();
		return FALSE;
	}

	return TRUE;
}

CString WinScpCommand::GetErrorString()
{
	return in->mErrorMsg;
}

HICON WinScpCommand::GetIcon()
{
	CString exeFilePath;
	if (in->mParam->ResolveExecutablePath(exeFilePath)) {
		return IconLoader::Get()->LoadIconFromPath(exeFilePath);
	}
	else {
		return IconLoader::Get()->LoadUnknownIcon();
	}
}

launcherapp::core::Command*
WinScpCommand::Clone()
{
	return new WinScpCommand(in->mParam, in->mSessionName);
}

CString WinScpCommand::TypeDisplayName()
{
	return _T("WinSCPセッション");
}

}}} // end of namespace launcherapp::commands::winscp
