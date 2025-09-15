#pragma once

#include "actions/core/ActionBase.h"
#include "commands/activate_window/WindowActivateMenuEventListener.h"

namespace launcherapp { namespace commands { namespace activate_window {

class TemporaryWindowNameAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	TemporaryWindowNameAction(HWND hwnd, MenuEventListener* listener);
	~TemporaryWindowNameAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param) override;

private:
	HWND mHwnd;
	MenuEventListener* mListener;
};



}}}

