#include "pch.h"
#include "RestoreWindowAction.h"
#include "utility/ScopeAttachThreadInput.h"

namespace launcherapp { namespace actions { namespace activate_window {

struct RestoreWindowAction::PImpl
{
	HWND mHwnd{nullptr};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


RestoreWindowAction::RestoreWindowAction() : in(new PImpl)
{
}

RestoreWindowAction::RestoreWindowAction(HWND hwnd) : in(new PImpl)
{
	in->mHwnd = hwnd;
}

RestoreWindowAction::~RestoreWindowAction()
{
}

// Action
// アクションの内容を示す名称
CString RestoreWindowAction::GetDisplayName()
{
	return _T("ウインドウ切り替え");
}

// アクションを実行する
bool RestoreWindowAction::Perform(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	ScopeAttachThreadInput scope;
	LONG_PTR style = GetWindowLongPtr(in->mHwnd, GWL_STYLE);

	if (style & WS_MINIMIZE) {
		// 最小化されていたら元に戻す
		PostMessage(in->mHwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	}

	SetForegroundWindow(in->mHwnd);
	return true;
}

}}}

