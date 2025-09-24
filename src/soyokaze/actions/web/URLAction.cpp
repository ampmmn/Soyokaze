#include "pch.h"
#include "URLAction.h"
#include "commands/common/SubProcess.h"
#include "actions/core/ActionParameter.h"

using namespace launcherapp::commands::common;
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;

namespace launcherapp { namespace actions { namespace web {

URLAction::URLAction(const CString& url) :
 	mDisplayName(_T("URLを開く")), mURL(url)
{
}

URLAction::URLAction(const CString& displayName, const CString& url) :
 	mDisplayName(displayName), mURL(url)
{
}

URLAction::~URLAction()
{
}

// Action
// アクションの内容を示す名称
CString URLAction::GetDisplayName()
{
	return mDisplayName;
}

// アクションを実行する
bool URLAction::Perform(Parameter* param, String* errMsg)
{
	UNREFERENCED_PARAMETER(param);

	SubProcess exec(ParameterBuilder::EmptyParam());
	SubProcess::ProcessPtr process;
	if (exec.Run(mURL, process) == FALSE) {
		if (errMsg) {
			UTF2UTF(process->GetErrorMessage(), *errMsg);
		}
		return false;
	}
	return true;
}


}}}

