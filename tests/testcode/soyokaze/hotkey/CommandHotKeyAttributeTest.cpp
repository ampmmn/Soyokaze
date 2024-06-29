#include "stdafx.h"
#include "gtest/gtest.h"
#include "hotkey/CommandHotKeyAttribute.h"

TEST(CommandHotKeyAttribute, testSizeOf)
{
	EXPECT_EQ(8, sizeof(CommandHotKeyAttribute));
}

TEST(CommandHotKeyAttribute, testConstruct)
{
	CommandHotKeyAttribute attr;
	EXPECT_EQ(-1, attr.mVirtualKeyIdx);
	EXPECT_FALSE(attr.mUseShift);
	EXPECT_FALSE(attr.mUseCtrl);
	EXPECT_FALSE(attr.mUseAlt);
	EXPECT_FALSE(attr.mUseWin);
}

TEST(CommandHotKeyAttribute, testCopyConstruct)
{
	CommandHotKeyAttribute attr;
	attr.mVirtualKeyIdx = 1;
	attr.mUseShift = true;
	attr.mUseCtrl = false;
	attr.mUseAlt = true;
	attr.mUseWin = false;

	CommandHotKeyAttribute attr2(attr);

	EXPECT_EQ(1, attr2.mVirtualKeyIdx);
	EXPECT_TRUE(attr2.mUseShift);
	EXPECT_FALSE(attr2.mUseCtrl);
	EXPECT_TRUE(attr2.mUseAlt);
	EXPECT_FALSE(attr2.mUseWin);
}

TEST(CommandHotKeyAttribute, testConstruct2)
{
	CommandHotKeyAttribute attr(MOD_SHIFT| MOD_ALT, 0x41);
	EXPECT_EQ(0, attr.mVirtualKeyIdx);
	EXPECT_TRUE(attr.mUseShift);
	EXPECT_FALSE(attr.mUseCtrl);
	EXPECT_TRUE(attr.mUseAlt);
	EXPECT_FALSE(attr.mUseWin);

	CommandHotKeyAttribute attr2(MOD_CONTROL | MOD_WIN, 0xdead);
	EXPECT_EQ(-1, attr2.mVirtualKeyIdx);
	EXPECT_FALSE(attr2.mUseShift);
	EXPECT_TRUE(attr2.mUseCtrl);
	EXPECT_FALSE(attr2.mUseAlt);
	EXPECT_TRUE(attr2.mUseWin);
}

TEST(CommandHotKeyAttribute, testEqual)
{
	CommandHotKeyAttribute attr(MOD_SHIFT| MOD_ALT, 0x41);
	EXPECT_TRUE(attr == attr);

	EXPECT_FALSE(attr == CommandHotKeyAttribute(MOD_ALT, 0x41));
	EXPECT_FALSE(attr == CommandHotKeyAttribute(MOD_SHIFT, 0x41));
	EXPECT_FALSE(attr == CommandHotKeyAttribute(MOD_SHIFT| MOD_ALT | MOD_CONTROL, 0x41));
	EXPECT_FALSE(attr == CommandHotKeyAttribute(MOD_SHIFT| MOD_ALT | MOD_WIN, 0x41));
	EXPECT_FALSE(attr == CommandHotKeyAttribute(MOD_SHIFT | MOD_ALT, 0x42));

}

TEST(CommandHotKeyAttribute, testNotEqual)
{
	CommandHotKeyAttribute attr(MOD_SHIFT| MOD_ALT, 0x41);
	EXPECT_FALSE(attr != attr);
}

TEST(CommandHotKeyAttribute, testIsInvalid)
{
	CommandHotKeyAttribute attr(MOD_SHIFT| MOD_ALT, -1);
	EXPECT_FALSE(attr.IsValid());
}

TEST(CommandHotKeyAttribute, testGetAccel)
{
	CommandHotKeyAttribute attr(MOD_SHIFT| MOD_ALT, -1);
	ACCEL accel;
	EXPECT_FALSE(attr.GetAccel(accel));

	CommandHotKeyAttribute attr2(MOD_SHIFT| MOD_ALT, 0x41);
	EXPECT_TRUE(attr2.GetAccel(accel));
}

TEST(CommandHotKeyAttribute, testCopy)
{
	CommandHotKeyAttribute attr(MOD_CONTROL | MOD_WIN, 0x45);
	CommandHotKeyAttribute attr2;

	attr2 = attr;


	EXPECT_EQ(4, attr2.mVirtualKeyIdx);
	EXPECT_FALSE(attr2.mUseShift);
	EXPECT_TRUE(attr2.mUseCtrl);
	EXPECT_FALSE(attr2.mUseAlt);
	EXPECT_TRUE(attr2.mUseWin);

}

TEST(CommandHotKeyAttribute, testLessThan1)
{
	CommandHotKeyAttribute attr(0, 0x41);

	EXPECT_FALSE(attr < CommandHotKeyAttribute(0, 0x41));

	EXPECT_TRUE(attr < CommandHotKeyAttribute(MOD_SHIFT, 0x41));
	EXPECT_FALSE(CommandHotKeyAttribute(MOD_SHIFT, 0x41) < attr);

	EXPECT_TRUE(attr < CommandHotKeyAttribute(MOD_CONTROL, 0x41));
	EXPECT_FALSE(CommandHotKeyAttribute(MOD_CONTROL, 0x41) < attr);

	EXPECT_TRUE(attr < CommandHotKeyAttribute(MOD_ALT, 0x41));
	EXPECT_FALSE(CommandHotKeyAttribute(MOD_ALT, 0x41) < attr);

	EXPECT_TRUE(attr < CommandHotKeyAttribute(MOD_WIN, 0x41));
	EXPECT_FALSE(CommandHotKeyAttribute(MOD_WIN, 0x41) < attr);

	EXPECT_TRUE(attr < CommandHotKeyAttribute(0, 0x42));
	EXPECT_FALSE(CommandHotKeyAttribute(0, 0x42) < attr);
}

