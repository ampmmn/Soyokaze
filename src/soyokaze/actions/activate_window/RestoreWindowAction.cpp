#include "pch.h"
#include "RestoreWindowAction.h"
#include "utility/ScopeAttachThreadInput.h"

namespace launcherapp { namespace actions { namespace activate_window {

struct RestoreWindowAction::PImpl
{
	std::unique_ptr<WindowTarget> mTarget;
	bool mIsSilent{false};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


RestoreWindowAction::RestoreWindowAction() : in(new PImpl)
{
}

RestoreWindowAction::RestoreWindowAction(HWND hwnd) : in(new PImpl)
{
	in->mTarget.reset(new SimpleWindowTarget(hwnd));
}

RestoreWindowAction::RestoreWindowAction(WindowTarget* target) : in(new PImpl)
{
	in->mTarget.reset(target);
}


RestoreWindowAction::~RestoreWindowAction()
{
}

void RestoreWindowAction::SetSilent(bool isSilent)
{
	in->mIsSilent = isSilent;
}


// Action
// アクションの内容を示す名称
CString RestoreWindowAction::GetDisplayName()
{
	return _T("ウインドウ切り替え");
}

// アクションを実行する
bool RestoreWindowAction::Perform(Parameter* param, String* errMsg)
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

	if (style & WS_MINIMIZE) {
		// 最小化されていたら元に戻す
		PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	}

	SetForegroundWindow(hwnd);
	return true;
}

}}}

