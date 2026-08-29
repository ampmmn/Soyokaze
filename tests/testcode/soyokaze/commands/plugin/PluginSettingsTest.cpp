#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/plugin/PluginSettings.h"

using launcherapp::commands::plugin::PluginSettings;

TEST(PluginSettingsTest, ReturnsDefaultValues)
{
	PluginSettings settings;
	auto item = settings.Get(_T("unknown"));
	EXPECT_TRUE(item.mIsEnabled);
	EXPECT_EQ(item.mPriority, 0);
}

TEST(PluginSettingsTest, ClampsPriorityWhenSetting)
{
	PluginSettings settings;
	settings.Set(_T("plugin"), {false, -10});
	auto item = settings.Get(_T("plugin"));
	EXPECT_FALSE(item.mIsEnabled);
	EXPECT_EQ(item.mPriority, 0);
}

TEST(PluginSettingsTest, ClonesValues)
{
	PluginSettings settings;
	settings.Set(_T("plugin"), {true, 10});
	auto clone = settings.Clone();
	auto item = clone->Get(_T("plugin"));
	EXPECT_TRUE(item.mIsEnabled);
	EXPECT_EQ(item.mPriority, 10);
}
