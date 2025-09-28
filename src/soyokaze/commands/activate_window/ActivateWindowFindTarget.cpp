#include "pch.h"
#include "ActivateWindowFindTarget.h"

namespace launcherapp { namespace commands { namespace activate_window {

constexpr int UPDATE_INTERVAL = 5000;


ActivateWindowFindTarget::ActivateWindowFindTarget() : 
	mCachedHwnd(nullptr), mLastUpdate(0)
{
}

ActivateWindowFindTarget::ActivateWindowFindTarget(const CommandParam& param) :
	mParam(param), mCachedHwnd(nullptr), mLastUpdate(0)
{
}

void ActivateWindowFindTarget::SetParam(const CommandParam& param)
{
	mParam = param;
	mCachedHwnd = nullptr;
	mLastUpdate = 0;
}

ActivateWindowFindTarget* ActivateWindowFindTarget::Clone()
{
	return new ActivateWindowFindTarget(mParam);
}

HWND ActivateWindowFindTarget::GetHandle()
{
	if (IsWindow(mCachedHwnd) && GetTickCount64() - mLastUpdate < UPDATE_INTERVAL) {
		return mCachedHwnd;
	}
	mCachedHwnd = nullptr;

	HWND hwnd = mParam.FindHwnd();
	if (IsWindow(hwnd) == FALSE) {
		return nullptr;
	}

	mCachedHwnd = hwnd;
	mLastUpdate = GetTickCount64();
	return mCachedHwnd;
}

}}}
