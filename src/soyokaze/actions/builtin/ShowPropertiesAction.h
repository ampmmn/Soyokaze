#pragma once

#include "actions/core/ActionBase.h"
#include "actions/builtin/ExecutionTarget.h"

namespace launcherapp { namespace actions { namespace builtin {

class ShowPropertiesAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	ShowPropertiesAction(const CString& filePath);
	ShowPropertiesAction(ExecutionTarget* target);
	~ShowPropertiesAction() = default;

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

