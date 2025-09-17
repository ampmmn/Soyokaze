#pragma once

#include "actions/core/ActionBase.h"
#include "actions/activate_window/CloseWindowAction.h"
#include "commands/activate_window/WindowActivateMenuEventListener.h"

namespace launcherapp { namespace commands { namespace activate_window {

// クローズしたことをMenuEventListener経由でProviderに通知するためのラッパークラス
class CloseWindowActionWrapper : virtual public launcherapp::actions::core::ActionBase
{
public:
	CloseWindowActionWrapper(HWND hwnd, MenuEventListener* listener);
	~CloseWindowActionWrapper();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	HWND mHwnd;
	MenuEventListener* mListener;
	launcherapp::actions::activate_window::CloseWindowAction mRealAction;
};



}}}

