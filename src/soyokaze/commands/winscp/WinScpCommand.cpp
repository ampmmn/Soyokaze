#include "pch.h"
#include "framework.h"
#include "WinScpCommand.h"
#include "commands/common/SubProcess.h"
#include "actions/builtin/CallbackAction.h"
#include "actions/core/ActionParameter.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using namespace launcherapp::actions::builtin;
using namespace launcherapp::actions::core;

namespace launcherapp { namespace commands { namespace winscp {

struct WinScpCommand::PImpl
{
	CommandParam* mParam{nullptr};
	CString mSessionName;
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

CString WinScpCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool WinScpCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags != 0) {
		return false;
	}

	*action = new CallbackAction(_T("セッションを開く"), [&](Parameter*, String* errMsg) -> bool {

		CString exeFilePath;
		if (in->mParam->ResolveExecutablePath(exeFilePath) == false) {
			spdlog::error("Failed to get winscp executable path");
			if (errMsg) { errMsg->assign("WinSCPの実行ファイルが見つかりません。"); }
			return false;
		}

		SubProcess exec(ParameterBuilder::EmptyParam());

		CString argStr;
		argStr.Format(_T("\"%s\""), (LPCTSTR)in->mSessionName);

		// プロセスを実行する
		SubProcess::ProcessPtr process;
		if (exec.Run(exeFilePath, argStr, process) == FALSE) {
			if (errMsg) {
				UTF2UTF(process->GetErrorMessage(), *errMsg);
			}
			return false;
		}
		return true;
	});

	return true;
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
