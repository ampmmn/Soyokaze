#include "SearchViewerCommandLine.h"

#include <gtest/gtest.h>

TEST(SearchViewerCommandLineTest, ParsesMultipleProfilesAndQuery)
{
	SearchViewerCommandLine::Options options;
	std::wstring error;
	ASSERT_TRUE(SearchViewerCommandLine::Parse({ L"--profile=google", L"--profile", L"bing", L"明日の天気" }, options, error)) << error;
	ASSERT_EQ(options.profiles.size(), 2U);
	EXPECT_EQ(options.profiles[0], L"google");
	EXPECT_EQ(options.profiles[1], L"bing");
	EXPECT_EQ(options.query, L"明日の天気");
}

TEST(SearchViewerCommandLineTest, ParsesConfigWithMultipleProfiles)
{
	SearchViewerCommandLine::Options options;
	std::wstring error;
	ASSERT_TRUE(SearchViewerCommandLine::Parse({ L"--config", L"Search Viewer.yaml", L"--profile", L"google", L"--profile=github", L"query" }, options, error)) << error;
	EXPECT_EQ(options.configPath, L"Search Viewer.yaml");
	ASSERT_EQ(options.profiles.size(), 2U);
}

TEST(SearchViewerCommandLineTest, RejectsDuplicateProfiles)
{
	SearchViewerCommandLine::Options options;
	std::wstring error;
	EXPECT_FALSE(SearchViewerCommandLine::Parse({ L"--profile=google", L"--profile", L"google", L"query" }, options, error));
	EXPECT_NE(error.find(L"google"), std::wstring::npos);
}

TEST(SearchViewerCommandLineTest, RejectsMissingProfilesAndQuery)
{
	SearchViewerCommandLine::Options options;
	std::wstring error;
	EXPECT_FALSE(SearchViewerCommandLine::Parse({ L"query" }, options, error));
	EXPECT_FALSE(SearchViewerCommandLine::Parse({ L"--profile=google" }, options, error));
}

TEST(SearchViewerCommandLineTest, QuotesWindowsArguments)
{
	EXPECT_EQ(SearchViewerCommandLine::QuoteArgument(L"plain"), L"\"plain\"");
	EXPECT_EQ(SearchViewerCommandLine::QuoteArgument(L"a\\\"b"), L"\"a\\\\\\\"b\"");
	EXPECT_EQ(SearchViewerCommandLine::QuoteArgument(L"path\\"), L"\"path\\\\\"");
}
