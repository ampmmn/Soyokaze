#include "stdafx.h"
#include "gtest/gtest.h"
#include "BreadCrumbs.h"

TEST(BreadCrumbs, canInit)
{
	BreadCrumbs crumbs(_T("A > B"));
	EXPECT_EQ(2, crumbs.GetDepth());
	EXPECT_STREQ(_T("A"), crumbs.GetItem(0));
	EXPECT_STREQ(_T("B"), crumbs.GetItem(1));
}

TEST(BreadCrumbs, ToString)
{
	BreadCrumbs crumbs(_T("A>B>C"));
	EXPECT_EQ(3, crumbs.GetDepth());
	EXPECT_STREQ(_T("A > B > C"), crumbs.ToString());
}
