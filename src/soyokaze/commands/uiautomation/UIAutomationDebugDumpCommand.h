#pragma once

#include "commands/common/AdhocCommandBase.h"

namespace launcherapp { namespace commands { namespace uiautomation {

class UIAutomationDebugDumpCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase
{
public:
	UIAutomationDebugDumpCommand(HWND hwnd);
	virtual ~UIAutomationDebugDumpCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(UIAutomationDebugDumpCommand)

protected:
	HWND mTargetWindow{nullptr};
};

}}} // end of namespace launcherapp::commands::uiautomation
