#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/shell_destinations/ShellDestinationCommand.h"
#include "actions/core/Action.h"
#include "hotkey/HotKeyAttribute.h"

using ShellDestinationCommand = launcherapp::commands::shell_destinations::ShellDestinationCommand;
using AutomaticDestinationEntry = launcherapp::commands::shell_destinations::AutomaticDestinationEntry;
using Action = launcherapp::actions::core::Action;

// ローカルパスの名前、説明、種類と追加アクションを取得できる
TEST(ShellDestinationCommandTest, LocalPathProvidesFileActions)
{
	AutomaticDestinationEntry entry{};
	entry.displayPath = L"C:\\Users\\test\\Report.xlsx";
	entry.lnk = { 1, 2, 3 };

	ShellDestinationCommand command(entry);
	EXPECT_EQ(_T("Report.xlsx"), command.GetName());
	EXPECT_EQ(_T("C:\\Users\\test\\Report.xlsx"), command.GetDescription());
	EXPECT_EQ(_T("ピン留めされた項目"), command.GetTypeDisplayName());
	EXPECT_EQ(3, command.GetMenuItemCount());

	Action* action = nullptr;
	EXPECT_TRUE(command.GetAction(HOTKEY_ATTR(MOD_CONTROL, VK_RETURN), &action));
	ASSERT_NE(nullptr, action);
	action->Release();

	action = nullptr;
	EXPECT_TRUE(command.GetAction(HOTKEY_ATTR(MOD_ALT, VK_RETURN), &action));
	ASSERT_NE(nullptr, action);
	action->Release();
}

// URLは通常実行のみ可能で、ローカルパス用の操作は提供しない
TEST(ShellDestinationCommandTest, UrlDoesNotProvideLocalPathActions)
{
	AutomaticDestinationEntry entry{};
	entry.displayPath = L"https://example.com/docs/Report.xlsx";
	entry.lnk = { 1, 2, 3 };

	ShellDestinationCommand command(entry);
	EXPECT_EQ(_T("Report.xlsx"), command.GetName());
	EXPECT_EQ(1, command.GetMenuItemCount());

	Action* action = nullptr;
	EXPECT_FALSE(command.GetAction(HOTKEY_ATTR(MOD_CONTROL, VK_RETURN), &action));
	EXPECT_EQ(nullptr, action);
}
