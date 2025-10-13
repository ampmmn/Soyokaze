#pragma once

#include "actions/core/ActionBase.h"
#include "commands/py_extension/PyExtensionCommandParam.h"

namespace launcherapp { namespace commands { namespace py_extension  {

class PyEvalAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	PyEvalAction(const CommandParam* param);
	~PyEvalAction();


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

