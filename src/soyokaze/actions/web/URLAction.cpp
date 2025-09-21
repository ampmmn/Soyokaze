#include "pch.h"
#include "URLAction.h"
#include "commands/common/SubProcess.h"

namespace launcherapp { namespace actions { namespace web {


using namespace launcherapp::commands::common;

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
	SubProcess exec(param);
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

