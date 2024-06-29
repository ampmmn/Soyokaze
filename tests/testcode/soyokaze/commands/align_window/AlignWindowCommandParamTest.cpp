#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/align_window/AlignWindowCommandParam.h"

using CommandParam = launcherapp::commands::align_window::CommandParam;
using ACTION = launcherapp::commands::align_window::ACTION;

TEST(AlignWindowCommandParam, testSizeOf)
{
	EXPECT_EQ(64, sizeof(CommandParam));
}

TEST(AlignWindowCommandParam, testConstruct)
{
	CommandParam param;
	EXPECT_FALSE(param.mIsNotifyIfWindowNotFound);
	EXPECT_TRUE(param.mIsKeepActiveWindow);
	EXPECT_TRUE(param.mName.IsEmpty());
	EXPECT_TRUE(param.mDescription.IsEmpty());
	EXPECT_TRUE(param.mItems.empty());
	EXPECT_TRUE(param.mHotKeyAttr == CommandHotKeyAttribute());
}

TEST(AlignWindowCommandParamItem, testSizeOf)
{
	EXPECT_EQ(44, sizeof(WINDOWPLACEMENT));
	EXPECT_EQ(88, sizeof(tregex));
	EXPECT_EQ(248, sizeof(CommandParam::ITEM));
}


TEST(AlignWindowCommandParamItem, testConstruct)
{
	CommandParam::ITEM item;
	EXPECT_TRUE(item.mCaptionStr.IsEmpty());
	EXPECT_TRUE(item.mClassStr.IsEmpty());
	EXPECT_FALSE(item.mIsUseRegExp);
	EXPECT_FALSE(item.mIsApplyAll);
	EXPECT_EQ(ACTION::AT_SETPOS, item.mAction);
}

TEST(AlignWindowCommandParamItem, testHasClassRegExpr)
{
	CommandParam::ITEM item;
	EXPECT_FALSE(item.HasClassRegExpr());

	item.mClassStr = _T("^aiueo");
	EXPECT_TRUE(item.HasClassRegExpr());
}

TEST(AlignWindowCommandParamItem, testHasCaptionRegExpr)
{
	CommandParam::ITEM item;
	EXPECT_FALSE(item.HasCaptionRegExpr());

	item.mCaptionStr = _T("^aiueo");
	EXPECT_TRUE(item.HasCaptionRegExpr());
}

TEST(AlignWindowCommandParamItem, testIsMatchClass)
{
	CommandParam::ITEM item;
	EXPECT_TRUE(item.IsMatchClass(_T("aiueo")));
	item.mClassStr = _T("aiueo");
	EXPECT_TRUE(item.IsMatchClass(_T("aiueo")));
	EXPECT_FALSE(item.IsMatchClass(_T("aiue")));
}

