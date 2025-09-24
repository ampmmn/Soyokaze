#pragma once

#include "actions/core/ActionBase.h"
#include "actions/builtin/ExecutionTarget.h"

namespace launcherapp { namespace actions { namespace builtin {

// 指定されたパスを実行するアクション
class ExecuteAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	enum {
		// 履歴に登録しない
		HISTORY_NONE,
		// 実行時引数があるときのみ履歴に登録する
		HISTORY_HASPARAMONLY,
		// 常に履歴に登録する
		HISTORY_ALWAYS,
	};
public:
	ExecuteAction(const CString& fullPath);
	ExecuteAction(const CString& fullPath, const CString& param, const CString& workDir = _T(""), int showType = SW_SHOW);
	ExecuteAction(ExecutionTarget* paramSrc);
	~ExecuteAction();

	void SetRunAsAdmin();
	bool SetAdditionalEnvironment(const CString& name, const CString& value);
	void SetHistoryPolicy(int policy);

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	void RegisterHistory(Parameter* param);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}}

