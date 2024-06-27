#include "stdafx.h"
#include "gtest/gtest.h"
#include "hotkey/HotKeyAttribute.h"

TEST(HotKeyAttribute, testSizeOf)
{
	EXPECT_EQ(6, sizeof(HOTKEY_ATTR));
}

TEST(HotKeyAttribute, testConstruct)
{
	HOTKEY_ATTR attr;
	EXPECT_EQ(-1, attr.mVirtualKeyIdx);
	EXPECT_FALSE(attr.mUseShift);
	EXPECT_FALSE(attr.mUseCtrl);
	EXPECT_FALSE(attr.mUseAlt);
	EXPECT_FALSE(attr.mUseWin);
}

TEST(HotKeyAttribute, testCopyConstruct)
{
	HOTKEY_ATTR attr;
	attr.mVirtualKeyIdx = 1;
	attr.mUseShift = true;
	attr.mUseCtrl = false;
	attr.mUseAlt = true;
	attr.mUseWin = false;

	HOTKEY_ATTR attr2(attr);

	EXPECT_EQ(1, attr2.mVirtualKeyIdx);
	EXPECT_TRUE(attr2.mUseShift);
	EXPECT_FALSE(attr2.mUseCtrl);
	EXPECT_TRUE(attr2.mUseAlt);
	EXPECT_FALSE(attr2.mUseWin);
}

TEST(HotKeyAttribute, testConstruct2)
{
	HOTKEY_ATTR attr(MOD_SHIFT| MOD_ALT, 0x41);
	EXPECT_EQ(0, attr.mVirtualKeyIdx);
	EXPECT_TRUE(attr.mUseShift);
	EXPECT_FALSE(attr.mUseCtrl);
	EXPECT_TRUE(attr.mUseAlt);
	EXPECT_FALSE(attr.mUseWin);

	HOTKEY_ATTR attr2(MOD_CONTROL | MOD_WIN, 0xdead);
	EXPECT_EQ(-1, attr2.mVirtualKeyIdx);
	EXPECT_FALSE(attr2.mUseShift);
	EXPECT_TRUE(attr2.mUseCtrl);
	EXPECT_FALSE(attr2.mUseAlt);
	EXPECT_TRUE(attr2.mUseWin);
}

TEST(HotKeyAttribute, testEqual)
{
	HOTKEY_ATTR attr(MOD_SHIFT| MOD_ALT, 0x41);
	EXPECT_TRUE(attr == attr);

	EXPECT_FALSE(attr == HOTKEY_ATTR(MOD_ALT, 0x41));
	EXPECT_FALSE(attr == HOTKEY_ATTR(MOD_SHIFT, 0x41));
	EXPECT_FALSE(attr == HOTKEY_ATTR(MOD_SHIFT| MOD_ALT | MOD_CONTROL, 0x41));
	EXPECT_FALSE(attr == HOTKEY_ATTR(MOD_SHIFT| MOD_ALT | MOD_WIN, 0x41));
	EXPECT_FALSE(attr == HOTKEY_ATTR(MOD_SHIFT | MOD_ALT, 0x42));

}

TEST(HotKeyAttribute, testNotEqual)
{
	HOTKEY_ATTR attr(MOD_SHIFT| MOD_ALT, 0x41);
	EXPECT_FALSE(attr != attr);
}

TEST(HotKeyAttribute, testIsInvalid)
{
	HOTKEY_ATTR attr(MOD_SHIFT| MOD_ALT, -1);
	EXPECT_FALSE(attr.IsValid());
}

TEST(HotKeyAttribute, testGetAccel)
{
	HOTKEY_ATTR attr(MOD_SHIFT| MOD_ALT, -1);
	ACCEL accel;
	EXPECT_FALSE(attr.GetAccel(accel));

	HOTKEY_ATTR attr2(MOD_SHIFT| MOD_ALT, 0x41);
	EXPECT_TRUE(attr2.GetAccel(accel));
}

TEST(HotKeyAttribute, testCopy)
{
	HOTKEY_ATTR attr(MOD_CONTROL | MOD_WIN, 0x45);
	HOTKEY_ATTR attr2;

	attr2 = attr;


	EXPECT_EQ(4, attr2.mVirtualKeyIdx);
	EXPECT_FALSE(attr2.mUseShift);
	EXPECT_TRUE(attr2.mUseCtrl);
	EXPECT_FALSE(attr2.mUseAlt);
	EXPECT_TRUE(attr2.mUseWin);

}

TEST(HotKeyAttribute, testLessThan1)
{
	HOTKEY_ATTR attr(0, 0x41);

	EXPECT_FALSE(attr < HOTKEY_ATTR(0, 0x41));

	EXPECT_TRUE(attr < HOTKEY_ATTR(MOD_SHIFT, 0x41));
	EXPECT_FALSE(HOTKEY_ATTR(MOD_SHIFT, 0x41) < attr);

	EXPECT_TRUE(attr < HOTKEY_ATTR(MOD_CONTROL, 0x41));
	EXPECT_FALSE(HOTKEY_ATTR(MOD_CONTROL, 0x41) < attr);

	EXPECT_TRUE(attr < HOTKEY_ATTR(MOD_ALT, 0x41));
	EXPECT_FALSE(HOTKEY_ATTR(MOD_ALT, 0x41) < attr);

	EXPECT_TRUE(attr < HOTKEY_ATTR(MOD_WIN, 0x41));
	EXPECT_FALSE(HOTKEY_ATTR(MOD_WIN, 0x41) < attr);

	EXPECT_TRUE(attr < HOTKEY_ATTR(0, 0x42));
	EXPECT_FALSE(HOTKEY_ATTR(0, 0x42) < attr);
}

