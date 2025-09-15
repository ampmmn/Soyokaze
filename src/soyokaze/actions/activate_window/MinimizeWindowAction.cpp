#include "pch.h"
#include "MinimizeWindowAction.h"
#include "utility/ScopeAttachThreadInput.h"

namespace launcherapp { namespace actions { namespace activate_window {

struct MinimizeWindowAction::PImpl
{
	std::unique_ptr<WindowTarget> mTarget;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


MinimizeWindowAction::MinimizeWindowAction() : in(new PImpl)
{
}

MinimizeWindowAction::MinimizeWindowAction(HWND hwnd) : in(new PImpl)
{
	in->mTarget.reset(new SimpleWindowTarget(hwnd));
}

MinimizeWindowAction::MinimizeWindowAction(WindowTarget* target) : in(new PImpl)
{
	in->mTarget.reset(target);
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

	if (in->mTarget.get() == nullptr) {
		return true;
	}
	auto hwnd = in->mTarget->GetHandle();
	if (IsWindow(hwnd) == FALSE) {
		return true;
	}

	ScopeAttachThreadInput scope;
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);

	if ((style & WS_MINIMIZE) == 0) {
		ShowWindow(hwnd, SW_MINIMIZE);
	}
	return true;
}

}}}

