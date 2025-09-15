#pragma once

#include "actions/core/ActionBase.h"
#include <memory>

namespace launcherapp { namespace actions { namespace activate_window {

class MinimizeWindowAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	MinimizeWindowAction();
	MinimizeWindowAction(HWND hwnd);
	~MinimizeWindowAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}}

