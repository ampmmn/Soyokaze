#include "pch.h"
#include "MaximizeWindowAction.h"
#include "utility/ScopeAttachThreadInput.h"

namespace launcherapp { namespace actions { namespace activate_window {

struct MaximizeWindowAction::PImpl
{
	HWND mHwnd{nullptr};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


MaximizeWindowAction::MaximizeWindowAction() : in(new PImpl)
{
}

MaximizeWindowAction::MaximizeWindowAction(HWND hwnd) : in(new PImpl)
{
	in->mHwnd = hwnd;
}

MaximizeWindowAction::~MaximizeWindowAction()
{
}

// Action
// アクションの内容を示す名称
CString MaximizeWindowAction::GetDisplayName()
{
	return _T("最大化");
}

// アクションを実行する
bool MaximizeWindowAction::Perform(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	ScopeAttachThreadInput scope;
	LONG_PTR style = GetWindowLongPtr(in->mHwnd, GWL_STYLE);

	if ((style & WS_MAXIMIZE) == 0) {
		PostMessage(in->mHwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	SetForegroundWindow(in->mHwnd);
	return true;
}

}}}

