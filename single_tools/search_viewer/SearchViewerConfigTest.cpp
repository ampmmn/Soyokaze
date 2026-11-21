#include "SearchViewerConfig.h"

#include <gtest/gtest.h>

#include <string>

namespace {

const std::string VALID_YAML =
	"profiles:\n"
	"  google:\n"
	"    site: search\n"
	"    title: Google検索\n"
	"sites:\n"
	"  search:\n"
	"    searchUrl: \"https://example.com/search?q={query}\"\n";

} // 名前空間

TEST(SearchViewerConfigTest, ParsesProfileAndBuildsEncodedUrl)
{
	SearchViewerConfig config;
	std::wstring error;
	ASSERT_TRUE(SearchViewerConfig::Parse(VALID_YAML, config, error)) << error;

	std::wstring title;
	ASSERT_TRUE(config.GetTitle(L"google", title));
	EXPECT_EQ(title, L"Google検索");

	std::wstring url;
	ASSERT_TRUE(config.BuildSearchUrl(L"google", L"明日の天気", url, error)) << error;
	EXPECT_EQ(url, L"https://example.com/search?q=%E6%98%8E%E6%97%A5%E3%81%AE%E5%A4%A9%E6%B0%97");
}

TEST(SearchViewerConfigTest, RejectsUnknownProfile)
{
	SearchViewerConfig config;
	std::wstring error;
	ASSERT_TRUE(SearchViewerConfig::Parse(VALID_YAML, config, error));

	std::wstring url;
	EXPECT_FALSE(config.BuildSearchUrl(L"missing", L"query", url, error));
	EXPECT_EQ(error, L"Unknown profile: missing");
}

TEST(SearchViewerConfigTest, RejectsUnknownSite)
{
	const std::string yaml =
		"profiles:\n"
		"  google:\n"
		"    site: missing\n"
		"    title: Google\n"
		"sites:\n"
		"  search:\n"
		"    searchUrl: \"https://example.com/?q={query}\"\n";
	SearchViewerConfig config;
	std::wstring error;
	EXPECT_FALSE(SearchViewerConfig::Parse(yaml, config, error));
	EXPECT_EQ(error, L"Unknown site: missing");
}

TEST(SearchViewerConfigTest, RejectsUnsafeSiteUrl)
{
	const std::string yaml =
		"profiles:\n"
		"  google:\n"
		"    site: search\n"
		"    title: Google\n"
		"sites:\n"
		"  search:\n"
		"    searchUrl: \"javascript:alert({query})\"\n";
	SearchViewerConfig config;
	std::wstring error;
	EXPECT_FALSE(SearchViewerConfig::Parse(yaml, config, error));
}

TEST(SearchViewerConfigTest, ValidatesProfileName)
{
	EXPECT_TRUE(SearchViewerConfig::IsValidProfileName(L"project-wiki1"));
	EXPECT_FALSE(SearchViewerConfig::IsValidProfileName(L"profile name"));
	EXPECT_FALSE(SearchViewerConfig::IsValidProfileName(L"12345678901234567"));
}

TEST(SearchViewerConfigTest, AllowsOnlyHttpAndHttpsUrls)
{
	EXPECT_TRUE(SearchViewerConfig::IsAllowedUrl(L"http://example.com/path"));
	EXPECT_TRUE(SearchViewerConfig::IsAllowedUrl(L"https://example.com/path"));
	EXPECT_FALSE(SearchViewerConfig::IsAllowedUrl(L"file:///C:/temp/file"));
	EXPECT_FALSE(SearchViewerConfig::IsAllowedUrl(L"https:///missing-host"));
}
