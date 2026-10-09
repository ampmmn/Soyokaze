#include "stdafx.h"
#include "gtest/gtest.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "hotkey/CommandHotKeyAttribute.h"

TEST(CommandHotKeyMappingsTest, EmptyMappingsAreEqual)
{
	CommandHotKeyMappings lhs;
	CommandHotKeyMappings rhs;
	EXPECT_TRUE(lhs == rhs);
	EXPECT_FALSE(lhs != rhs);
}

TEST(CommandHotKeyMappingsTest, SameContentsAreEqual)
{
	CommandHotKeyMappings lhs;
	CommandHotKeyMappings rhs;
	lhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	rhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	EXPECT_TRUE(lhs == rhs);
	EXPECT_FALSE(lhs != rhs);
}

TEST(CommandHotKeyMappingsTest, DifferentItemCountIsNotEqual)
{
	CommandHotKeyMappings lhs;
	CommandHotKeyMappings rhs;
	lhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	EXPECT_FALSE(lhs == rhs);
	EXPECT_TRUE(lhs != rhs);

	rhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	rhs.AddItem(_T("bravo"), CommandHotKeyAttribute(MOD_CONTROL, VK_F2));
	EXPECT_FALSE(lhs == rhs);
	EXPECT_TRUE(lhs != rhs);
}

TEST(CommandHotKeyMappingsTest, DifferentNameIsNotEqual)
{
	CommandHotKeyMappings lhs;
	CommandHotKeyMappings rhs;
	lhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	rhs.AddItem(_T("bravo"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	EXPECT_FALSE(lhs == rhs);
	EXPECT_TRUE(lhs != rhs);
}

TEST(CommandHotKeyMappingsTest, DifferentAttributeIsNotEqual)
{
	CommandHotKeyMappings lhs;
	CommandHotKeyMappings rhs;
	lhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	rhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F2));
	EXPECT_FALSE(lhs == rhs);
	EXPECT_TRUE(lhs != rhs);
}

TEST(CommandHotKeyMappingsTest, GlobalFlagDifferenceIsNotEqual)
{
	CommandHotKeyMappings lhs;
	CommandHotKeyMappings rhs;
	lhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1, false));
	rhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1, true));
	EXPECT_FALSE(lhs == rhs);
}

TEST(CommandHotKeyMappingsTest, OrderIsIgnored)
{
	CommandHotKeyMappings lhs;
	CommandHotKeyMappings rhs;
	lhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	lhs.AddItem(_T("bravo"), CommandHotKeyAttribute(MOD_CONTROL, VK_F2));
	rhs.AddItem(_T("bravo"), CommandHotKeyAttribute(MOD_CONTROL, VK_F2));
	rhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	EXPECT_TRUE(lhs == rhs);
	EXPECT_FALSE(lhs != rhs);
}

TEST(CommandHotKeyMappingsTest, ComparisonIsSymmetric)
{
	CommandHotKeyMappings lhs;
	CommandHotKeyMappings rhs;
	lhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	rhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F2));
	EXPECT_EQ(lhs == rhs, rhs == lhs);
}

TEST(CommandHotKeyMappingsTest, ConstructedMappingsAreEmpty)
{
	CommandHotKeyMappings mappings;
	EXPECT_EQ(0, mappings.GetItemCount());
}

TEST(CommandHotKeyMappingsTest, AddItemThenGetNameAndAttr)
{
	CommandHotKeyMappings mappings;
	CommandHotKeyAttribute attr1(MOD_CONTROL, VK_F1);
	CommandHotKeyAttribute attr2(MOD_SHIFT, VK_F2, true);

	// 追加した順に件数・名前・属性が取り出せることを確認する
	mappings.AddItem(_T("alpha"), attr1);
	mappings.AddItem(_T("bravo"), attr2);
	EXPECT_EQ(2, mappings.GetItemCount());
	EXPECT_TRUE(mappings.GetName(0) == _T("alpha"));
	EXPECT_TRUE(mappings.GetName(1) == _T("bravo"));

	CommandHotKeyAttribute got;
	mappings.GetHotKeyAttr(1, got);
	EXPECT_TRUE(got == attr2);
	EXPECT_TRUE(got.IsGlobal());
}

TEST(CommandHotKeyMappingsTest, RemoveItemExistingName)
{
	CommandHotKeyMappings mappings;
	mappings.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	mappings.AddItem(_T("bravo"), CommandHotKeyAttribute(MOD_CONTROL, VK_F2));
	mappings.AddItem(_T("charlie"), CommandHotKeyAttribute(MOD_CONTROL, VK_F3));

	// 存在する名前は削除されて true を返し、残りの順序は保たれる
	EXPECT_TRUE(mappings.RemoveItem(_T("bravo")));
	EXPECT_EQ(2, mappings.GetItemCount());
	EXPECT_TRUE(mappings.GetName(0) == _T("alpha"));
	EXPECT_TRUE(mappings.GetName(1) == _T("charlie"));
}

TEST(CommandHotKeyMappingsTest, RemoveItemNotExistingName)
{
	CommandHotKeyMappings mappings;
	mappings.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));

	// 存在しない名前は false を返し、件数は変わらない
	EXPECT_FALSE(mappings.RemoveItem(_T("delta")));
	EXPECT_EQ(1, mappings.GetItemCount());
}

TEST(CommandHotKeyMappingsTest, FindKeyMappingStringExistingName)
{
	CommandHotKeyMappings mappings;
	CommandHotKeyAttribute attr(MOD_CONTROL, VK_F1);
	mappings.AddItem(_T("alpha"), attr);

	// 存在する名前は属性の表示用文字列を返す
	EXPECT_TRUE(mappings.FindKeyMappingString(_T("alpha")) == attr.ToString());
}

TEST(CommandHotKeyMappingsTest, FindKeyMappingStringNotExistingName)
{
	CommandHotKeyMappings mappings;
	mappings.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));

	// 存在しない名前は空文字列を返す
	EXPECT_TRUE(mappings.FindKeyMappingString(_T("delta")).IsEmpty());
}

TEST(CommandHotKeyMappingsTest, SwapExchangesContents)
{
	CommandHotKeyMappings lhs;
	CommandHotKeyMappings rhs;
	lhs.AddItem(_T("alpha"), CommandHotKeyAttribute(MOD_CONTROL, VK_F1));
	rhs.AddItem(_T("bravo"), CommandHotKeyAttribute(MOD_CONTROL, VK_F2));
	rhs.AddItem(_T("charlie"), CommandHotKeyAttribute(MOD_CONTROL, VK_F3));

	// 両者の内容が入れ替わることを確認する
	lhs.Swap(rhs);
	EXPECT_EQ(2, lhs.GetItemCount());
	EXPECT_TRUE(lhs.GetName(0) == _T("bravo"));
	EXPECT_EQ(1, rhs.GetItemCount());
	EXPECT_TRUE(rhs.GetName(0) == _T("alpha"));
}
