#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/group/CommandParam.h"
#include "commands/transfer/CommandJSONEntry.h"

using launcherapp::commands::group::CommandParam;
using launcherapp::commands::group::GroupItemType;
using launcherapp::commands::transfer::CommandJSONEntry;

TEST(GroupCommandParamTest, SaveAndLoadItems)
{
	CommandParam param;
	param.mName = _T("group");
	launcherapp::commands::group::GroupItem item;
	item.mType = GroupItemType::Path;
	item.mItemName = _T("tool.exe");
	item.mParam = _T("$1");
	item.mShowType = SW_HIDE;
	item.mWorkDir = _T("C:\\Tools");
	item.mIsWait = true;
	param.mItems.push_back(item);

	CommandJSONEntry entry(_T("group"));
	entry.Init();
	ASSERT_TRUE(param.Save(&entry));

	CommandParam loaded;
	ASSERT_TRUE(loaded.Load(&entry));
	ASSERT_EQ(loaded.mItems.size(), 1U);
	EXPECT_EQ(loaded.mItems[0].mType, GroupItemType::Path);
	EXPECT_EQ(loaded.mItems[0].mItemName, _T("tool.exe"));
	EXPECT_EQ(loaded.mItems[0].mParam, _T("$1"));
	EXPECT_EQ(loaded.mItems[0].mShowType, SW_HIDE);
	EXPECT_EQ(loaded.mItems[0].mWorkDir, _T("C:\\Tools"));
	EXPECT_TRUE(loaded.mItems[0].mIsWait);
}

TEST(GroupCommandParamTest, LoadLegacyPassParam)
{
	CommandJSONEntry entry(_T("group"));
	entry.Init();
	entry.Set(_T("IsPassParam"), true);
	entry.Set(_T("CommandCount"), 1);
	entry.Set(_T("ItemName1"), _T("command"));

	CommandParam loaded;
	ASSERT_TRUE(loaded.Load(&entry));
	ASSERT_EQ(loaded.mItems.size(), 1U);
	EXPECT_EQ(loaded.mItems[0].mType, GroupItemType::Command);
	EXPECT_EQ(loaded.mItems[0].mParam, _T("$*"));
}
