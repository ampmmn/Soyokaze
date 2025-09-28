#pragma once

#include "actions/activate_window/ActivateWindowTarget.h"
#include "actions/core/ActionBase.h"
#include <memory>

namespace launcherapp { namespace actions { namespace activate_window {

class RestoreWindowAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	RestoreWindowAction();
	RestoreWindowAction(HWND hwnd);
	RestoreWindowAction(WindowTarget* target);
	~RestoreWindowAction();

	void SetSilent(bool isSilent);

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

