#pragma once

#include "actions/core/ActionBase.h"
#include <memory>

namespace launcherapp { namespace actions { namespace activate_window {

class RestoreWindowAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	RestoreWindowAction();
	RestoreWindowAction(HWND hwnd);
	~RestoreWindowAction();

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

