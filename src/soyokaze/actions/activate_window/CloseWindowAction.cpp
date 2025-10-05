#include "pch.h"
#include "CloseWindowAction.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/TimeoutChecker.h"

namespace launcherapp { namespace actions { namespace activate_window {

struct CloseWindowAction::PImpl
{
	std::unique_ptr<WindowTarget> mTarget;
	bool mIsSilent{false};
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

void CloseWindowAction::SetSilent(bool isSilent)
{
	in->mIsSilent = isSilent;
}

// Action
// アクションの内容を示す名称
CString CloseWindowAction::GetDisplayName()
{
	return _T("閉じる");
}

// アクションを実行する
bool CloseWindowAction::Perform(Parameter* param, String* errMsg)
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

	SendMessage(hwnd, WM_CLOSE, 0, 0);

	// ウインドウが閉じるまでまつ
	launcherapp::utility::TimeoutChecker tm(500);
	while(IsWindow(hwnd) && tm.IsTimeout() == false) {
		Sleep(50);
	}

	return true;
}

}}}

