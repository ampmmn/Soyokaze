#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/everything/EverythingAdhocCommand.h"
#include "commands/everything/EverythingCommandParam.h"
#include "commands/everything/EverythingResult.h"
#include "commands/core/ContextMenuSourceIF.h"

using EverythingAdhocCommand = launcherapp::commands::everything::EverythingAdhocCommand;
using CommandParam = launcherapp::commands::everything::CommandParam;
using EverythingResult = launcherapp::commands::everything::EverythingResult;
using ContextMenuSource = launcherapp::commands::core::ContextMenuSource;
using Action = launcherapp::actions::core::Action;

TEST(EverythingAdhocCommandTest, ProvidesContextMenuActions)
{
	EverythingAdhocCommand command(CommandParam(), EverythingResult(_T("C:\\example.txt")));
	ContextMenuSource* menuSource = nullptr;
	ASSERT_TRUE(command.QueryInterface(IFID_CONTEXTMENUSOURCE, (void**)&menuSource));
	ASSERT_NE(nullptr, menuSource);

	EXPECT_EQ(5, menuSource->GetMenuItemCount());

	const std::vector<CString> expectedNames = {
		_T("実行"),
		_T("パスを開く"),
		_T("プログラムから開く"),
		_T("クリップボードにコピー"),
		_T("プロパティ"),
	};
	for (int i = 0; i < (int)expectedNames.size(); ++i) {
		Action* action = nullptr;
		ASSERT_TRUE(menuSource->GetMenuItem(i, &action));
		ASSERT_NE(nullptr, action);
		EXPECT_EQ(expectedNames[i], action->GetDisplayName());
		action->Release();
	}

	Action* action = nullptr;
	EXPECT_FALSE(menuSource->GetMenuItem(-1, &action));
	EXPECT_FALSE(menuSource->GetMenuItem(5, &action));
	menuSource->Release();
}
