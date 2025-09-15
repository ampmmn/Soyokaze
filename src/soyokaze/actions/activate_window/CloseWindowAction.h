#pragma once

#include "actions/activate_window/ActivateWindowTarget.h"
#include "actions/core/ActionBase.h"
#include <memory>

namespace launcherapp { namespace actions { namespace activate_window {

class CloseWindowAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	CloseWindowAction();
	CloseWindowAction(HWND hwnd);
	CloseWindowAction(WindowTarget* target);
	~CloseWindowAction();

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

