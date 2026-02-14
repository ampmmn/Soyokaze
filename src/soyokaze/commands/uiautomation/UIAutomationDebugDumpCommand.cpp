#include "pch.h"
#include "framework.h"
#include "UIAutomationDebugDumpCommand.h"
#include "commands/uiautomation/WindowUIElements.h"
#include "actions/builtin/CallbackAction.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::actions::builtin;

namespace launcherapp { namespace commands { namespace uiautomation {

using namespace launcherapp::commands::common;

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(UIAutomationDebugDumpCommand)

UIAutomationDebugDumpCommand::UIAutomationDebugDumpCommand(
	HWND hwnd
) : mTargetWindow(hwnd)
{
	this->mName = _T("uiautomation-debug-dump");
	this->mDescription = _T("アクティブウインドウのUIAutomationElementの構造をデバッグ出力");
}

UIAutomationDebugDumpCommand::~UIAutomationDebugDumpCommand()
{
}

CString UIAutomationDebugDumpCommand::GetTypeDisplayName()
{
	return _T("UI要素(デバッグ)");
}

bool UIAutomationDebugDumpCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}

	*action = new CallbackAction(_T("uiautomation.jsonをログフォルダーに出力"), [&](Parameter*, String*) -> bool {

			// デバッグ用出力
			WindowUIElements windowUIElements(mTargetWindow);
			windowUIElements.Dump();

			return true;
	});

	return true;
}

HICON UIAutomationDebugDumpCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromHwnd(mTargetWindow);
}

launcherapp::core::Command*
UIAutomationDebugDumpCommand::Clone()
{
	return new UIAutomationDebugDumpCommand(mTargetWindow);
}


}}} // end of namespace launcherapp::commands::uiautomation
