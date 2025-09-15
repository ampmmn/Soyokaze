#pragma once

#include "actions/activate_window/ActivateWindowTarget.h"
#include "commands/activate_window/WindowActivateCommandParam.h"

namespace launcherapp { namespace commands { namespace activate_window {

class ActivateWindowFindTarget : public launcherapp::actions::activate_window::WindowTarget
{
public:
	ActivateWindowFindTarget();
	ActivateWindowFindTarget(const CommandParam& param);
	void SetParam(const CommandParam& param);
	ActivateWindowFindTarget* Clone();

	HWND FindHwnd(bool isShowErrMsg);


	HWND GetHandle() override;

	CommandParam mParam;
	HWND mCachedHwnd;
	uint64_t mLastUpdate;
};

}}}




