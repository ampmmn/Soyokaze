#include "pch.h"
#include "ExecuteAction.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/common/CommandParameterFunctions.h"
#include <map>

namespace launcherapp { namespace actions { namespace builtin {


using namespace launcherapp::commands::common;
using namespace launcherapp::actions::core;

// たんにExecuteActionのコントストラクタで受け取ったパスとパラメータを使うための実装
class DefaultExecutionTarget : public ExecutionTarget
{
public:
	DefaultExecutionTarget(const CString& fullPath, const CString& paramStr, const CString& workDir, int showType) : 
		mFullPath(fullPath), mParameter(paramStr), mWorkDir(workDir), mShowType(showType) {}
	~DefaultExecutionTarget() {}

	CString GetPath(Parameter*) override {
		return mFullPath;
	}

	CString GetParameter(Parameter*) override {
		return mParameter;
	}

	CString GetWorkDir(Parameter*) override {
		return mWorkDir;
	}

	int GetShowType(Parameter*) override {
		return mShowType;
	}

	CString mFullPath;
	CString mParameter;
	CString mWorkDir;
	int mShowType{SW_SHOW};
};

struct ExecuteAction::PImpl
{
	std::unique_ptr<ExecutionTarget> mTarget;
	bool mShouldRunAsAdmin{false};
	int mHistoryPolicy{ExecuteAction::HISTORY_NONE};
	std::map<CString, CString> mEnvMap;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


ExecuteAction::ExecuteAction(const CString& fullPath) : in(new PImpl)
{
	in->mTarget.reset(new DefaultExecutionTarget(fullPath, _T(""), _T(""), SW_SHOW));
}

ExecuteAction::ExecuteAction(
	const CString& fullPath,
 	const CString& param,
 	const CString& workDir,
 	int showType
) : in(new PImpl)
{
	in->mTarget.reset(new DefaultExecutionTarget(fullPath, param, workDir, showType));
}

ExecuteAction::ExecuteAction(ExecutionTarget* target) : in(new PImpl)
{
	in->mTarget.reset(target);
}

ExecuteAction::~ExecuteAction()
{
}

void ExecuteAction::SetRunAsAdmin()
{
	in->mShouldRunAsAdmin = true;
}

bool ExecuteAction::SetAdditionalEnvironment(const CString& name, const CString& value)
{
	in->mEnvMap[name] = value;
	return true;
}

void ExecuteAction::SetHistoryPolicy(int policy)
{
	in->mHistoryPolicy = policy; 
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
	// 履歴に登録
	RegisterHistory(args_);

	// 表示方法
	SubProcess exec(args_);
	exec.SetShowType(in->mTarget->GetShowType(args_));

	// 管理者権限で実行するかどうか
	if (in->mShouldRunAsAdmin) {
		exec.SetRunAsAdmin();
	}

	// 作業ディレクトリ
	auto workDir = in->mTarget->GetWorkDir(args_);
	if (workDir.IsEmpty() == FALSE) {
		exec.SetWorkDirectory(workDir);
	}

	// 追加の環境変数
	for (auto& item : in->mEnvMap) {
		exec.SetAdditionalEnvironment(item.first, item.second);
	}

	// プロセスを起動
	SubProcess::ProcessPtr process;
	if (exec.Run(in->mTarget->GetPath(args_), in->mTarget->GetParameter(args_), process) == false) {
		if (errMsg) {
			UTF2UTF(process->GetErrorMessage(), *errMsg);
		}
		return false;
	}

	// もしwaitするようにするのであればここで待つ
	auto namedParam = GetNamedParameter(args_);
	if (namedParam->GetNamedParamBool(_T("WAIT"))) {
		const int WAIT_LIMIT = 30 * 1000; // 30 seconds.
		process->Wait(WAIT_LIMIT);
	}

	return true;
}

void ExecuteAction::RegisterHistory(Parameter* param)
{
	int policy =in->mHistoryPolicy; 
	if (policy == ExecuteAction::HISTORY_NONE) {
		// 履歴に登録しない
		return;
	}

	auto namedParam = GetNamedParameter(param);
	if (namedParam->GetNamedParamBool(_T("RunAsHistory"))) {
		// アクションが履歴経由で実行されているときは再登録しない
		return;
	}

	if (policy == ExecuteAction::HISTORY_HASPARAMONLY && param->HasParameter() == false) {
		// 実行時引数があるときのみ保存する設定で、かつ、実行時引数がない場合は登録スキップ
		return;
	}

	// 実行履歴に登録する
	ExecuteHistory::GetInstance()->Add(_T("history"), param->GetWholeString());
}

}}}

