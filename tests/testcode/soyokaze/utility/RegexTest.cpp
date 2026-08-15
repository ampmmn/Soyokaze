#include "stdafx.h"
#include "utility/Regex.h"

TEST(RegexTest, FullMatch)
{
	launcherapp::utility::Regex regex(_T("^([0-9]+)\\.([0-9]+)$"));
	std::vector<CString> captures;

	EXPECT_TRUE(regex.FullMatch(_T("12.34"), captures));
	ASSERT_EQ(2, captures.size());
	EXPECT_STREQ(_T("12"), captures[0]);
	EXPECT_STREQ(_T("34"), captures[1]);
	EXPECT_FALSE(regex.FullMatch(_T("12.34.56")));
}

TEST(RegexTest, PartialMatch)
{
	launcherapp::utility::Regex regex(_T("https?://.+"));

	EXPECT_TRUE(regex.PartialMatch(_T("open https://example.com")));
	EXPECT_FALSE(regex.PartialMatch(_T("example.com")));
}

TEST(RegexTest, InvalidPattern)
{
	launcherapp::utility::Regex regex;

	EXPECT_FALSE(regex.Compile(_T("(")));
	EXPECT_FALSE(regex.IsValid());
	EXPECT_FALSE(regex.GetError().empty());
}
