#include "pch.h"
#include "TemporaryWindowNameAction.h"

namespace launcherapp { namespace commands { namespace activate_window {

TemporaryWindowNameAction::TemporaryWindowNameAction(HWND hwnd, MenuEventListener* listener) :
	mHwnd(hwnd), mListener(listener)
{
}

TemporaryWindowNameAction::~TemporaryWindowNameAction()
{
}

// Action
// アクションの内容を示す名称
CString TemporaryWindowNameAction::GetDisplayName()
{
	return _T("ウインドウに一時的な名前を付ける");
}

// アクションを実行する
bool TemporaryWindowNameAction::Perform(Parameter* param, String* errMsg)
{
	UNREFERENCED_PARAMETER(param);
	UNREFERENCED_PARAMETER(errMsg);

	if (mListener) {
		mListener->OnRequestPutName(mHwnd);
	}
	return true;
}

}}}

