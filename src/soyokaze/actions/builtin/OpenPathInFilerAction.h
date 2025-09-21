#pragma once

#include "actions/core/ActionBase.h"
#include "actions/builtin/ExecutionTarget.h"
#include <memory>

namespace launcherapp { namespace actions { namespace builtin {

// 指定されたパスをファイラで開くアクション
class OpenPathInFilerAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	OpenPathInFilerAction(const CString fullPath);
	OpenPathInFilerAction(ExecutionTarget* target);
	~OpenPathInFilerAction();

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

