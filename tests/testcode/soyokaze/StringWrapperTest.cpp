#include "stdafx.h"
#include "gtest/gtest.h"
#include "launcherapp_string.h"
#include <cwchar>

TEST(StringTest, ConstructFromStdString) {
    std::string s = "Hello";
    String str(s);
    EXPECT_EQ(str, "Hello");
}

TEST(StringTest, ConstructFromWchar) {
    const wchar_t* ws = L"こんにちは";
    String str(ws);
    // UTF-8で"こんにちは"は15バイト
    EXPECT_EQ(str, u8"こんにちは");
}

TEST(StringTest, MakeLower) {
    String str("HeLLo123");
    str.MakeLower();
    EXPECT_EQ(str, "hello123");
}

TEST(StringTest, TrimLeft) {
    String str("   \t\nHello  ");
    str.TrimLeft();
    EXPECT_EQ(str, "Hello  ");

    String str2("NoSpace");
    str2.TrimLeft();
    EXPECT_EQ(str2, "NoSpace");

    String str3("   ");
    str3.TrimLeft();
    EXPECT_EQ(str3, "");
}

TEST(StringTest, CompareNoCase) {
    String str("AbCdE");
    EXPECT_EQ(str.CompareNoCase("abcde"), 0);
    EXPECT_EQ(str.CompareNoCase("ABCDE"), 0);
    EXPECT_EQ(str.CompareNoCase("abcdef"), -1);
    EXPECT_EQ(str.CompareNoCase("abcd"), 1);
}

TEST(StringTest, Left) {
    String str("abcdef");
    EXPECT_EQ(str.Left(3), "abc");
    EXPECT_EQ(str.Left(0), "");
    EXPECT_EQ(str.Left(10), "abcdef");
}

TEST(StringTest, MidWithLen) {
    String str("abcdef");
    EXPECT_EQ(str.Mid(2, 3), "cde");
    EXPECT_EQ(str.Mid(0, 2), "ab");
    EXPECT_EQ(str.Mid(4, 10), "ef");
    EXPECT_EQ(str.Mid(10, 2), "");
}

TEST(StringTest, MidWithoutLen) {
    String str("abcdef");
    EXPECT_EQ(str.Mid(2), "cdef");
    EXPECT_EQ(str.Mid(0), "abcdef");
    EXPECT_EQ(str.Mid(6), "");
    EXPECT_EQ(str.Mid(10), "");
}

TEST(StringTest, FindCString) {
    String str("abcdefabc");
    EXPECT_EQ(str.Find("abc"), 0);
    EXPECT_EQ(str.Find("abc", 1), 6);
    EXPECT_EQ(str.Find("def"), 3);
    EXPECT_EQ(str.Find("xyz"), -1);
}

TEST(StringTest, FindStdString) {
    String str("abcdefabc");
    EXPECT_EQ(str.Find(std::string("abc")), 0);
    EXPECT_EQ(str.Find(std::string("abc"), 1), 6);
    EXPECT_EQ(str.Find(std::string("def")), 3);
    EXPECT_EQ(str.Find(std::string("xyz")), -1);
}

TEST(StringTest, FindChar) {
    String str("abcdefabc");
    EXPECT_EQ(str.Find('a'), 0);
    EXPECT_EQ(str.Find('a', 1), 6);
    EXPECT_EQ(str.Find('z'), -1);
}
