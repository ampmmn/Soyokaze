#include "stdafx.h"
#include "gtest/gtest.h"
#include "icon/AppIcon.h"

using AppIcon = launcherapp::icon::AppIcon;

class AppIconTest : public ::testing::Test {
	protected:
		AppIcon* icon;
		void SetUp() override {
			icon = AppIcon::Get();
			icon->Reset();
		}
		void TearDown() override {
			// 破棄はIconLoaderのシングルトンに任せる
		}
};

TEST_F(AppIconTest, GetInstanceReturnsSingleton) {
	auto icon2 = AppIcon::Get();
	EXPECT_EQ(icon, icon2);
}

TEST_F(AppIconTest, DefaultIconHandleShouldReturnsSameIcon) {
	HICON h = icon->DefaultIconHandle();
	HICON h2 = icon->DefaultIconHandle();
	EXPECT_EQ(h, h2);
}

TEST_F(AppIconTest, Reset) {
	icon->Reset();
}





