#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/snippetgroup/SnippetGroupParam.h"
#include "commands/transfer/CommandJSONEntry.h"

using launcherapp::commands::snippetgroup::SnippetGroupParam;
using launcherapp::commands::transfer::CommandJSONEntry;

TEST(SnippetGroupParamTest, SaveAndLoadIconData)
{
	SnippetGroupParam param;
	param.mName = _T("snippet");
	param.mDescription = _T("description");
	param.mIconData = { 0x01, 0x02, 0x03, 0x04 };

	CommandJSONEntry entry(_T("snippet"));
	entry.Init();
	ASSERT_TRUE(param.Save(&entry));

	SnippetGroupParam loaded;
	ASSERT_TRUE(loaded.Load(&entry));

	EXPECT_EQ(param.mName, loaded.mName);
	EXPECT_EQ(param.mDescription, loaded.mDescription);
	EXPECT_EQ(param.mIconData, loaded.mIconData);
	EXPECT_TRUE(loaded.mItems.empty());
}

TEST(SnippetGroupParamTest, LoadWithoutIconData)
{
	CommandJSONEntry entry(_T("snippet"));
	entry.Init();
	entry.Set(_T("description"), _T("description"));
	entry.Set(_T("ItemCount"), 0);

	SnippetGroupParam loaded;
	loaded.mIconData = { 0x01 };
	ASSERT_TRUE(loaded.Load(&entry));

	EXPECT_TRUE(loaded.mIconData.empty());
}
