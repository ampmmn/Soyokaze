#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/websearch/SearchViewerProxy.h"

using launcherapp::commands::websearch::SearchViewerProxy;

TEST(SearchViewerProxyTest, EmptyTitleUsesCommandName)
{
	EXPECT_EQ(_T("Googleの検索結果"), SearchViewerProxy::CreateWindowTitle(_T("Google"), _T("")));
}

TEST(SearchViewerProxyTest, ExplicitTitleIsPreserved)
{
	EXPECT_EQ(_T("任意のタイトル"), SearchViewerProxy::CreateWindowTitle(_T("Google"), _T("任意のタイトル")));
}
