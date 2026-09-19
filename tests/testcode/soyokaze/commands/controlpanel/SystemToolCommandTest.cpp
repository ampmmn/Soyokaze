#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/controlpanel/SystemToolCommand.h"

using SystemToolCommand = launcherapp::commands::controlpanel::SystemToolCommand;
using Action = launcherapp::actions::core::Action;

TEST(SystemToolCommandTest, ProvidesCommandInformation)
{
	std::vector<CString> commandLine = {_T("rundll32.exe"), _T("example.dll,EntryPoint")};
	SystemToolCommand command(_T("表示名"), _T("説明"), commandLine);

	EXPECT_EQ(_T("表示名"), command.GetName());
	EXPECT_EQ(_T("説明"), command.GetDescription());
	EXPECT_EQ(_T("システムツール"), command.GetTypeDisplayName());
	EXPECT_EQ(1, command.GetMenuItemCount());
}

TEST(SystemToolCommandTest, CreatesActionOnlyForPlainExecution)
{
	std::vector<CString> commandLine = {_T("rundll32.exe")};
	SystemToolCommand command(_T("表示名"), _T("説明"), commandLine);
	Action* action = nullptr;

	EXPECT_TRUE(command.GetAction(HOTKEY_ATTR(0, VK_RETURN), &action));
	ASSERT_NE(nullptr, action);
	action->Release();

	EXPECT_FALSE(command.GetAction(HOTKEY_ATTR(MOD_SHIFT, VK_RETURN), &action));
}
