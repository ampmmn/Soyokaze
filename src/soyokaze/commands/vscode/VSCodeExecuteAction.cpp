#include "pch.h"
#include "VSCodeExecuteAction.h"
#include "commands/vscode/VSCodeCommandParam.h"
#include "actions/builtin/ExecuteAction.h"

using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;

namespace launcherapp { namespace commands { namespace vscode {

class CommandParam;

VSCodeExecuteAction::VSCodeExecuteAction(
	const CommandParam* param,
 	const CString& path,
 	bool isOpenInNewWindow
) : mParam(param), mFullPath(path), mIsOpenInNewWindow(isOpenInNewWindow)
{
}

VSCodeExecuteAction::~VSCodeExecuteAction()
{
}

CString VSCodeExecuteAction::GetDisplayName()
{
	return mIsOpenInNewWindow ? _T("新しいウインドウで開く") : _T("開く");

}

// アクションを実行する
bool VSCodeExecuteAction::Perform(Parameter* param, String* errMsg)
{
	CString actionArg;
	if (mIsOpenInNewWindow) {
		actionArg += _T("-n ");
	}
	else {
		actionArg += _T("-r ");
	}

	actionArg += _T("\"");
	actionArg += mFullPath;
	actionArg += _T("\"");

	ExecuteAction action(mParam->GetVSCodeExePath(), actionArg);
	action.SetHistoryPolicy(ExecuteAction::HISTORY_NONE);
	return action.Perform(param, errMsg);
}


}}} // end of namespace launcherapp::commands::vscode


