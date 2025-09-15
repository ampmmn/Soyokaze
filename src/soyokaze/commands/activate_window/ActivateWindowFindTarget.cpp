#include "pch.h"
#include "ActivateWindowFindTarget.h"
#include "commands/common/Message.h"

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

HWND ActivateWindowFindTarget::FindHwnd(bool isShowErrMsg)
{
	if (IsWindow(mCachedHwnd) && GetTickCount64() - mLastUpdate < UPDATE_INTERVAL) {
		return mCachedHwnd;
	}
	mCachedHwnd = nullptr;

	HWND hwnd = mParam.FindHwnd();
	if (IsWindow(hwnd) == FALSE) {

		if (isShowErrMsg) {
			launcherapp::commands::common::PopupMessage(_T("指定されたウインドウが見つかりません"));
		}
		return nullptr;
	}

	mCachedHwnd = hwnd;
	mLastUpdate = GetTickCount64();
	return mCachedHwnd;
}

HWND ActivateWindowFindTarget::GetHandle()
{
	return FindHwnd(mParam.mIsNotifyIfWindowNotFound);
}

}}}
