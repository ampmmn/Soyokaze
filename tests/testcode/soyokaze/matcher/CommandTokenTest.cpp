#include "stdafx.h"
#include "gtest/gtest.h"
#include "matcher/CommandToken.h"

using CommandToken = launcherapp::matcher::CommandToken;

TEST(CommandToken, constructor)
{
	CommandToken tok(_T("a b"));
	EXPECT_EQ(2, tok.GetCount());
}

TEST(CommandToken, destructor)
{
	CommandToken tok(_T("a b"));
}

TEST(CommandToken, TestGetCount)
{
	CommandToken tok(_T("a b"));
	EXPECT_EQ(2, tok.GetCount());
}

TEST(CommandToken, GetTrailingString1)
{
	CommandToken tok(_T("a b"));

	CString text;
	bool result = tok.GetTrailingString(1, text);
	EXPECT_TRUE(result);
	EXPECT_EQ(_T("b"), text);
}

TEST(CommandToken, GetTrailingString2)
{
	CommandToken tok(_T("a b"));

	CString text;
	bool result = tok.GetTrailingString(0, text);
	EXPECT_TRUE(result);
	EXPECT_EQ(_T("a b"), text);
}

TEST(CommandToken, GetTrailingString3)
{
	CommandToken tok(_T("a b"));

	CString text;
	bool result = tok.GetTrailingString(3, text);
	EXPECT_FALSE(result);
}
