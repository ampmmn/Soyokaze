#include "pch.h"
#include "MaximizeWindowAction.h"
#include "utility/ScopeAttachThreadInput.h"

namespace launcherapp { namespace actions { namespace activate_window {

struct MaximizeWindowAction::PImpl
{
	std::unique_ptr<WindowTarget> mTarget;
	bool mIsSilent{false};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


MaximizeWindowAction::MaximizeWindowAction() : in(new PImpl)
{
}

MaximizeWindowAction::MaximizeWindowAction(HWND hwnd) : in(new PImpl)
{
	in->mTarget.reset(new SimpleWindowTarget(hwnd));
}

MaximizeWindowAction::MaximizeWindowAction(WindowTarget* target) : in(new PImpl)
{
	in->mTarget.reset(target);
}

MaximizeWindowAction::~MaximizeWindowAction()
{
}

void MaximizeWindowAction::SetSilent(bool isSilent)
{
	in->mIsSilent = isSilent;
}


// Action
// アクションの内容を示す名称
CString MaximizeWindowAction::GetDisplayName()
{
	return _T("最大化");
}

// アクションを実行する
bool MaximizeWindowAction::Perform(Parameter* param, String* errMsg)
{
	UNREFERENCED_PARAMETER(param);

	if (in->mTarget.get() == nullptr) {
		return true;
	}
	auto hwnd = in->mTarget->GetHandle();
	if (IsWindow(hwnd) == FALSE) {

		if (in->mIsSilent == false && errMsg) {
			*errMsg = "指定されたウインドウが見つかりません";
		}

		return false;
	}

	ScopeAttachThreadInput scope;
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);

	if ((style & WS_MAXIMIZE) == 0) {
		PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	SetForegroundWindow(hwnd);
	return true;
}

}}}

