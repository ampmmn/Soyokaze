#include "pch.h"
#include "CloseWindowAction.h"
#include "utility/ScopeAttachThreadInput.h"

namespace launcherapp { namespace actions { namespace activate_window {

struct CloseWindowAction::PImpl
{
	std::unique_ptr<WindowTarget> mTarget;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


CloseWindowAction::CloseWindowAction() : in(new PImpl)
{
}

CloseWindowAction::CloseWindowAction(HWND hwnd) : in(new PImpl)
{
	in->mTarget.reset(new SimpleWindowTarget(hwnd));
}

CloseWindowAction::CloseWindowAction(WindowTarget* target) : in(new PImpl)
{
	in->mTarget.reset(target);
}

CloseWindowAction::~CloseWindowAction()
{
}

// Action
// アクションの内容を示す名称
CString CloseWindowAction::GetDisplayName()
{
	return _T("閉じる");
}

// アクションを実行する
bool CloseWindowAction::Perform(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	if (in->mTarget.get() == nullptr) {
		return true;
	}
	auto hwnd = in->mTarget->GetHandle();
	if (IsWindow(hwnd) == FALSE) {
		return true;
	}

	PostMessage(hwnd, WM_CLOSE, 0, 0);
	return true;
}

}}}

