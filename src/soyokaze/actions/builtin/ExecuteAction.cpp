#include "pch.h"
#include "ExecuteAction.h"
#include "commands/common/SubProcess.h"
#include <map>

namespace launcherapp { namespace actions { namespace builtin {


using namespace launcherapp::commands::common;
using namespace launcherapp::actions::core;


struct ExecuteAction::PImpl
{
	CString mFullPath;
	CString mParameter;
	CString mWorkDir;
	int mShowType{SW_SHOW};
	bool mShouldRunAsAdmin{false};
	std::map<CString, CString> mEnvMap;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



ExecuteAction::ExecuteAction(const CString& fullPath, const CString& param) : in(new PImpl)
{
 	in->mFullPath = fullPath;
	in->mParameter = param;
}

ExecuteAction::~ExecuteAction()
{
}

void ExecuteAction::SetShowType(int showType)
{
	in->mShowType = showType;
}

void ExecuteAction::SetRunAsAdmin()
{
	in->mShouldRunAsAdmin = true;
}

void ExecuteAction::SetWorkDirectory(const CString& dir)
{
	in->mWorkDir = dir;
}

bool ExecuteAction::SetAdditionalEnvironment(const CString& name, const CString& value)
{
	in->mEnvMap[name] = value;
	return true;
}


// Action
// アクションの内容を示す名称
CString ExecuteAction::GetDisplayName()
{
	if (in->mShouldRunAsAdmin) {
		return _T("管理者権限で実行");
	}
	else {
		return _T("実行");
	}
}

// アクションを実行する
bool ExecuteAction::Perform(Parameter* args_, String* errMsg)
{
	SubProcess exec(args_);
	exec.SetShowType(in->mShowType);
	if (in->mShouldRunAsAdmin) {
		exec.SetRunAsAdmin();
	}
	if (in->mWorkDir.IsEmpty() == FALSE) {
		exec.SetWorkDirectory(in->mWorkDir);
	}
	for (auto& item : in->mEnvMap) {
		exec.SetAdditionalEnvironment(item.first, item.second);
	}

	SubProcess::ProcessPtr process;
	if (exec.Run(in->mFullPath, in->mParameter, process) == false) {
		if (errMsg) {
			UTF2UTF(process->GetErrorMessage(), *errMsg);
		}
		return false;
	}
	return true;
}

}}}

