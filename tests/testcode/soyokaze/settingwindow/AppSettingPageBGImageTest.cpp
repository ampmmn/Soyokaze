#include "stdafx.h"
#include "gtest/gtest.h"
#include "settingwindow/AppSettingPageBGImage.h"

TEST(AppSettingPageBGImage, HasExpectedPageInformation)
{
	AppSettingPageBGImage page;

	EXPECT_EQ(CString(_T("表示\\背景画像")), page.GetPagePath());
	EXPECT_EQ(CString(_T("背景画像")), page.GetName());
	EXPECT_EQ(60, page.GetOrder());
}

TEST(AppSettingPageBGImage, CanClone)
{
	AppSettingPageBGImage page;
	std::unique_ptr<launcherapp::settingwindow::AppSettingPageIF> cloned(page.Clone());

	ASSERT_NE(nullptr, cloned);
	EXPECT_EQ(page.GetPagePath(), cloned->GetPagePath());
}
