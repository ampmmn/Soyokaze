#include "pch.h"
#include "MinimizeWindowAction.h"
#include "utility/ScopeAttachThreadInput.h"

namespace launcherapp { namespace actions { namespace activate_window {

struct MinimizeWindowAction::PImpl
{
	HWND mHwnd{nullptr};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


MinimizeWindowAction::MinimizeWindowAction() : in(new PImpl)
{
}

MinimizeWindowAction::MinimizeWindowAction(HWND hwnd) : in(new PImpl)
{
	in->mHwnd = hwnd;
}

MinimizeWindowAction::~MinimizeWindowAction()
{
}

// Action
// アクションの内容を示す名称
CString MinimizeWindowAction::GetDisplayName()
{
	return _T("最小化");
}

// アクションを実行する
bool MinimizeWindowAction::Perform(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	ScopeAttachThreadInput scope;
	LONG_PTR style = GetWindowLongPtr(in->mHwnd, GWL_STYLE);

	if ((style & WS_MINIMIZE) == 0) {
		ShowWindow(in->mHwnd, SW_MINIMIZE);
	}
	return true;
}

}}}

