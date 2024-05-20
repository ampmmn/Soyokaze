#include "stdafx.h"
#include "gtest/gtest.h"
#include "macros/core/Token.h"

using Token = launcherapp::macros::core::Token;

TEST(MacrosCoreToken, testConstruct)
{
	Token tok(_T("aiueo"));
}

TEST(MacrosCoreToken, testAtEnd)
{
	Token tok(_T("aiueo"));
	EXPECT_FALSE(tok.AtEnd());
	
	Token tok2(_T(""));
	EXPECT_TRUE(tok2.AtEnd());
}

TEST(MacrosCoreToken, testNext)
{
	Token tok(_T("ai"));
	EXPECT_EQ(0, tok.GetPos());
	tok.Next();
	EXPECT_EQ(1, tok.GetPos());
	tok.Next();
	EXPECT_EQ(2, tok.GetPos());
	tok.Next();
	EXPECT_EQ(2, tok.GetPos());
	EXPECT_TRUE(tok.AtEnd());
}

TEST(MacrosCoreToken, testGet)
{
	Token tok(_T("ai"));
	EXPECT_EQ(_T('a'), tok.Get());
	tok.Next();
	EXPECT_EQ(_T('i'), tok.Get());
}

TEST(MacrosCoreToken, testIsWhiteSpace)
{
	Token tok(_T("a \ti"));
	EXPECT_FALSE(tok.IsWhiteSpace());
	tok.Next();
	EXPECT_TRUE(tok.IsWhiteSpace());
	tok.Next();
	EXPECT_TRUE(tok.IsWhiteSpace());
	tok.Next();
	EXPECT_FALSE(tok.IsWhiteSpace());
	tok.Next();
	EXPECT_FALSE(tok.IsWhiteSpace());
}

TEST(MacrosCoreToken, testSkipWhiteSpace)
{
	Token tok(_T("   hoge"));
	tok.SkipWhiteSpace();
	EXPECT_EQ(3, tok.GetPos());

	tok.Next();
	tok.Next();
	tok.Next();
	tok.Next();
	tok.SkipWhiteSpace();
	EXPECT_EQ(7, tok.GetPos());
}


TEST(MacrosCoreToken, testSkipName)
{
	Token tok(_T("ai hoge"));
	CString name;
	tok.SkipName(name);
	EXPECT_TRUE(name == _T("ai"));

	EXPECT_EQ(2, tok.GetPos());
}

TEST(MacrosCoreToken, testSkipName2)
{
	Token tok(_T("ai}"));
	CString name;
	tok.SkipName(name);
	EXPECT_TRUE(name == _T("ai"));
}

TEST(MacrosCoreToken, testSkipUntil)
{
	Token tok(_T("   hoge"));
	tok.SkipUntil(_T('h'));
	EXPECT_EQ(3, tok.GetPos());
	tok.SkipUntil(_T('g'));
	EXPECT_EQ(5, tok.GetPos());
	tok.SkipUntil(_T('}'));
	EXPECT_EQ(7, tok.GetPos());
}


TEST(MacrosCoreToken, testSkipString)
{
	Token tok(_T("aiueo   hoge"));

	CString str;
	tok.SkipString(str);
	EXPECT_EQ(5, tok.GetPos());
	EXPECT_TRUE(str == _T("aiueo"));
}

TEST(MacrosCoreToken, testSkipString2)
{
	Token tok(_T("hoge}"));

	CString str;
	tok.SkipString(str);
	EXPECT_EQ(4, tok.GetPos());
	EXPECT_TRUE(str == _T("hoge"));
}

TEST(MacrosCoreToken, testSkipString3)
{
	Token tok(_T("hoge\\ }"));

	CString str;
	tok.SkipString(str);
	EXPECT_EQ(6, tok.GetPos());
	EXPECT_TRUE(str == _T("hoge\\ "));
}

TEST(MacrosCoreToken, testSkipString4)
{
	Token tok(_T("hoge\\} }"));

	CString str;
	tok.SkipString(str);
	EXPECT_EQ(6, tok.GetPos());
	EXPECT_TRUE(str == _T("hoge}"));
}
