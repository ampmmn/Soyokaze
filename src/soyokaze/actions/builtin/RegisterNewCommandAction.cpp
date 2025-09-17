#include "pch.h"
#include "RegisterNewCommandAction.h"
#include "commands/common/Clipboard.h"
#include "actions/core/ActionParameter.h"
#include "commands/core/CommandRepository.h"

namespace launcherapp { namespace actions { namespace builtin {

using namespace launcherapp::actions::core;

// Action
// アクションの内容を示す名称
CString RegisterNewCommandAction::GetDisplayName()
{
	return _T("コマンドを登録");
}

// アクションを実行する
bool RegisterNewCommandAction::Perform(Parameter* param, String* errMsg)
{
	UNREFERENCED_PARAMETER(errMsg);

	RefPtr<ParameterBuilder> inParam(ParameterBuilder::Create(), false);

	bool hasParam = false;

	int paramCount = param->GetParamCount();
	if (paramCount > 0) {
		inParam->SetNamedParamString(_T("COMMAND"), param->GetParam(0));
		hasParam = true;
	}

	if (paramCount > 1) {
		inParam->SetNamedParamString(_T("PATH"), param->GetParam(1));
		hasParam = true;
	}
	if (hasParam) {
		inParam->SetNamedParamString(_T("TYPE"), _T("ShellExecuteCommand"));
	}

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->NewCommandDialog(inParam);

	return true;
}

}}}

