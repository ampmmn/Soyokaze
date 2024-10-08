#include "pch.h"
#include "framework.h"
#include "WindowActivateAdhocCommand.h"
#include "utility/ScopeAttachThreadInput.h"
#include "commands/common/CommandParameterFunctions.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_window {

using namespace launcherapp::commands::common;

struct WindowActivateAdhocCommand::PImpl
{
	HWND mHwnd;
};


WindowActivateAdhocCommand::WindowActivateAdhocCommand(
	HWND hwnd
) : in(std::make_unique<PImpl>())
{
	in->mHwnd = hwnd;

	TCHAR caption[256];
	GetWindowText(hwnd, caption, 256);
	this->mName = caption;
	this->mDescription = caption;
}

WindowActivateAdhocCommand::~WindowActivateAdhocCommand()
{
}

CString WindowActivateAdhocCommand::GetGuideString()
{
	return _T("Enter:ウインドウをアクティブにする");
}

CString WindowActivateAdhocCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_WINDOWACTIVATE);
	return TEXT_TYPE;
}

BOOL WindowActivateAdhocCommand::Execute(Parameter* param)
{
	ScopeAttachThreadInput scope;

	bool isCtrlKeyPressed = GetModifierKeyState(param, MASK_CTRL) != 0;

	LONG_PTR style = GetWindowLongPtr(in->mHwnd, GWL_STYLE);
	if (isCtrlKeyPressed && (style & WS_MAXIMIZE) == 0) {
		// Ctrlキーが押されていたら最大化表示する
		PostMessage(in->mHwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	else if (style & WS_MINIMIZE) {
		// 最小化されていたら元に戻す
		PostMessage(in->mHwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	}

	SetForegroundWindow(in->mHwnd);
	return TRUE;
}

HICON WindowActivateAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromHwnd(in->mHwnd);
}

launcherapp::core::Command*
WindowActivateAdhocCommand::Clone()
{
	return new WindowActivateAdhocCommand(in->mHwnd);
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

