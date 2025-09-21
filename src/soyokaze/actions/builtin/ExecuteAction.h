#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace builtin {

// 指定されたパスを実行するアクション
class ExecuteAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	ExecuteAction(const CString& fullPath, const CString& param);
	~ExecuteAction();

	void SetShowType(int showType);
	void SetRunAsAdmin();
	void SetWorkDirectory(const CString& dir);
	bool SetAdditionalEnvironment(const CString& name, const CString& value);

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}}

