#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/websearch/SearchViewerWindow.h"

using launcherapp::commands::websearch::SearchViewerWindow;

TEST(SearchViewerWindowTest, AllowsOnlyHttpAndHttpsUrls)
{
	EXPECT_TRUE(SearchViewerWindow::IsAllowedUrl(_T("http://example.com")));
	EXPECT_TRUE(SearchViewerWindow::IsAllowedUrl(_T("HTTPS://example.com")));
	EXPECT_FALSE(SearchViewerWindow::IsAllowedUrl(_T("file:///C:/test.html")));
	EXPECT_FALSE(SearchViewerWindow::IsAllowedUrl(_T("mailto:user@example.com")));
	EXPECT_FALSE(SearchViewerWindow::IsAllowedUrl(_T("https:")));
}

TEST(SearchViewerWindowTest, PlacementNameIsAsciiAndUniqueForUnicodeCommandNames)
{
	const CString japaneseName = SearchViewerWindow::CreateWindowPlacementName(_T("あ"));
	const CString englishName = SearchViewerWindow::CreateWindowPlacementName(_T("a"));

	EXPECT_EQ(_T("SearchViewer-3042"), japaneseName);
	EXPECT_NE(japaneseName, englishName);
	for (int i = 0; i < japaneseName.GetLength(); ++i) {
		EXPECT_LE(japaneseName[i], _T('~'));
	}
}

TEST(SearchViewerWindowTest, RestoresMaximizedStateButNotMinimizedState)
{
	WINDOWPLACEMENT placement{};
	placement.showCmd = SW_SHOWMAXIMIZED;
	EXPECT_EQ(SW_SHOWMAXIMIZED, SearchViewerWindow::GetRestoreShowCommand(placement));

	placement.showCmd = SW_SHOWNORMAL;
	EXPECT_EQ(SW_SHOW, SearchViewerWindow::GetRestoreShowCommand(placement));

	placement.showCmd = SW_SHOWMINIMIZED;
	EXPECT_EQ(SW_SHOW, SearchViewerWindow::GetRestoreShowCommand(placement));
}
