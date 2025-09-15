#include "pch.h"
#include "CloseWindowAction.h"
#include "utility/ScopeAttachThreadInput.h"

namespace launcherapp { namespace actions { namespace activate_window {

struct CloseWindowAction::PImpl
{
	HWND mHwnd{nullptr};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


CloseWindowAction::CloseWindowAction() : in(new PImpl)
{
}

CloseWindowAction::CloseWindowAction(HWND hwnd) : in(new PImpl)
{
	in->mHwnd = hwnd;
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

	PostMessage(in->mHwnd, WM_CLOSE, 0, 0);
	return true;
}

}}}

