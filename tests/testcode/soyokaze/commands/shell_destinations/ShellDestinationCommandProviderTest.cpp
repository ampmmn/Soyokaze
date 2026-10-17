#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/shell_destinations/ShellDestinationCommandProvider.h"

using ShellDestinationCommandProvider = launcherapp::commands::shell_destinations::ShellDestinationCommandProvider;

// Providerの識別名と表示するコマンド種別を取得できる
TEST(ShellDestinationCommandProviderTest, ProvidesCommandDisplayName)
{
	ShellDestinationCommandProvider provider;
	EXPECT_EQ(_T("ShellDestinationCommand"), provider.GetName());

	std::vector<CString> displayNames;
	EXPECT_EQ(1, provider.EnumCommandDisplayNames(displayNames));
	ASSERT_EQ(1, displayNames.size());
	EXPECT_EQ(_T("ピン留めされた項目"), displayNames[0]);
}
