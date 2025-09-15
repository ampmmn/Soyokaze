#include "pch.h"
#include "CloseWindowActionWrapper.h"

namespace launcherapp { namespace commands { namespace activate_window {

CloseWindowActionWrapper::CloseWindowActionWrapper(HWND hwnd, MenuEventListener* listener) : 
	mHwnd(hwnd), mListener(listener), mRealAction(hwnd)
{
}

CloseWindowActionWrapper::~CloseWindowActionWrapper()
{
}

// Action
// アクションの内容を示す名称
CString CloseWindowActionWrapper::GetDisplayName()
{
	return mRealAction.GetDisplayName();
}

// アクションを実行する
bool CloseWindowActionWrapper::Perform(Parameter* param)
{
	bool result = mRealAction.Perform(param);

	if (mListener) {
		mListener->OnRequestClose(mHwnd);
	}
	return result;
}

}}}

