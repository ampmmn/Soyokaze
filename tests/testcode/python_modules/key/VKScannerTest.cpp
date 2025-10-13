#include "stdafx.h"
#include <gtest/gtest.h>
#include "VKScanner.h"

TEST(VKScannerTest, HasNext_ReturnsFalseOnEmptyInput)
{
    VKScanner scanner("");
    EXPECT_FALSE(scanner.HasNext());
}

TEST(VKScannerTest, HasNext_ReturnsTrueOnValidInput)
{
    VKScanner scanner("A");
    EXPECT_TRUE(scanner.HasNext());
}

TEST(VKScannerTest, Get_ReturnsFalseOnEmptyInput)
{
    VKScanner scanner("");
    KEY_DEFINE key;
    EXPECT_FALSE(scanner.Get(&key));
}

TEST(VKScannerTest, Next_AdvancesPosition)
{
    VKScanner scanner("A B");
    KEY_DEFINE key;
    EXPECT_TRUE(scanner.HasNext());
    EXPECT_TRUE(scanner.Next(&key));
    EXPECT_EQ("a", key.mKeyName);
    EXPECT_EQ(0x41, key.mVK);
    EXPECT_TRUE(scanner.HasNext());
    EXPECT_TRUE(scanner.Next(&key));
    EXPECT_EQ(" ", key.mKeyName);
    EXPECT_EQ(VK_SPACE, key.mVK);
    EXPECT_TRUE(scanner.HasNext());
    EXPECT_TRUE(scanner.Next(&key));
    EXPECT_EQ("b", key.mKeyName);
    EXPECT_EQ(0x42, key.mVK);
}

TEST(VKScannerTest, Get_ReturnsCurrentKey)
{
    VKScanner scanner("A B");
    KEY_DEFINE key;
    EXPECT_TRUE(scanner.Get(&key));
}

