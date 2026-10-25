#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommand.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandParam.h"

using launcherapp::commands::url_directoryindex::URLDirectoryIndexCommand;
using launcherapp::commands::url_directoryindex::CommandParam;

namespace {

/**
  UTF-8文字列をHTMLバイト列に変換する
  @param[in] html UTF-8形式のHTML
  @return HTMLのバイト列
*/
std::vector<BYTE> MakeHTMLContent(const std::string& html)
{
	return std::vector<BYTE>(html.begin(), html.end());
}

}

// Lexborでアンカーを抽出し、最終URLを基準に同一オリジン判定すること
TEST(URLDirectoryIndexCommandTest, ExtractHTMLLinks_UsesFinalURLOrigin)
{
	const std::string html =
		"<html><body>"
		"<a href=\"child/\">Child</a>"
		"<a href=\"?download=1\">Query</a>"
		"<a href=\"HTTPS://EXAMPLE.test:8443/absolute\">Absolute</a>"
		"<a href=\"//example.test:8443/network\">Protocol Relative</a>"
		"<a href=\"../\">Parent Directory</a>"
		"<a href=\"https://example.test:9443/external\">Other Port</a>"
		"<a href=\"http://example.test:8443/external\">Other Scheme</a>"
		"<a href=\"https://other.test:8443/external\">Other Host</a>"
		"<a href=\"?C=N;O=D\">Sort</a>"
		"</body></html>";
	std::vector<std::pair<CString, CString>> links;

	ASSERT_TRUE(URLDirectoryIndexCommand::ExtractHTMLLinks(
		MakeHTMLContent(html), _T("https://example.test:8443/dir/index.html"), links));
	ASSERT_EQ(5u, links.size());
	EXPECT_STREQ(_T("https://example.test:8443/dir/child/"), (LPCTSTR)links[0].first);
	EXPECT_STREQ(_T("Child"), (LPCTSTR)links[0].second);
	EXPECT_STREQ(_T("https://example.test:8443/dir/index.html?download=1"), (LPCTSTR)links[1].first);
	EXPECT_STREQ(_T("Query"), (LPCTSTR)links[1].second);
	EXPECT_STREQ(_T("HTTPS://EXAMPLE.test:8443/absolute"), (LPCTSTR)links[2].first);
	EXPECT_STREQ(_T("Absolute"), (LPCTSTR)links[2].second);
	CString combinedAbsoluteURL = CommandParam::CombineURL2(_T("https://example.test:8443/dir/"), links[2].first);
	EXPECT_STREQ(_T("HTTPS://EXAMPLE.test:8443/absolute"), (LPCTSTR)combinedAbsoluteURL);
	EXPECT_STREQ(_T("https://example.test:8443/network"), (LPCTSTR)links[3].first);
	EXPECT_STREQ(_T("Protocol Relative"), (LPCTSTR)links[3].second);
	EXPECT_STREQ(_T("https://example.test:8443/"), (LPCTSTR)links[4].first);
	EXPECT_STREQ(_T(".."), (LPCTSTR)links[4].second);
}
