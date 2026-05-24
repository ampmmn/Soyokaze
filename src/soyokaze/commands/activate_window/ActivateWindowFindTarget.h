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

	// ハンドルを取得する(キャッシュの結果を返す場合がある)
	HWND GetHandle() override;
	// ハンドルを取得する(キャッシュを使わない)
	HWND FetchHandle();


	CommandParam mParam;
	HWND mCachedHwnd;
	uint64_t mLastUpdate;
};

}}}




