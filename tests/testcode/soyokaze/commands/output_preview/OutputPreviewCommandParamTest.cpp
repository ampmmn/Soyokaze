#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/output_preview/OutputPreviewCommandParam.h"
#include "commands/transfer/CommandJSONEntry.h"

using launcherapp::commands::output_preview::OutputPreviewParam;
using launcherapp::commands::transfer::CommandJSONEntry;

TEST(OutputPreviewCommandParamTest, SavesAndLoadsAllSettings)
{
	OutputPreviewParam param;
	param.mName = _T("preview");
	param.mDescription = _T("description");
	param.mFilePath = _T("C:\\Tools\\tool.exe");
	param.mParameter = _T("--input $1 --output $output");
	param.mWorkDir = _T("C:\\Tools");
	param.mShowType = 2;
	param.mResultType = 1;
	param.mExtension = _T(".html");
	param.mPreviewType = 1;
	param.mTimeout = 2500;
	param.mForce = 1;

	CommandJSONEntry entry(_T("preview"));
	entry.Init();
	ASSERT_TRUE(param.Save(&entry));

	OutputPreviewParam loaded;
	ASSERT_TRUE(loaded.Load(&entry));
	EXPECT_EQ(loaded.mName, param.mName);
	EXPECT_EQ(loaded.mDescription, param.mDescription);
	EXPECT_EQ(loaded.mFilePath, param.mFilePath);
	EXPECT_EQ(loaded.mParameter, param.mParameter);
	EXPECT_EQ(loaded.mWorkDir, param.mWorkDir);
	EXPECT_EQ(loaded.mShowType, param.mShowType);
	EXPECT_EQ(loaded.mResultType, param.mResultType);
	EXPECT_EQ(loaded.mExtension, param.mExtension);
	EXPECT_EQ(loaded.mPreviewType, param.mPreviewType);
	EXPECT_EQ(loaded.mTimeout, param.mTimeout);
	EXPECT_EQ(loaded.mForce, param.mForce);
}

TEST(OutputPreviewCommandParamTest, UsesDefaultExtensionAndTimeoutForOldEntries)
{
	CommandJSONEntry entry(_T("preview"));
	entry.Init();

	OutputPreviewParam loaded;
	ASSERT_TRUE(loaded.Load(&entry));
	EXPECT_EQ(loaded.mExtension, _T(".txt"));
	EXPECT_EQ(loaded.mTimeout, 500);
	EXPECT_EQ(loaded.mForce, 0);
}

TEST(OutputPreviewCommandParamTest, AllowsNonzeroExitCodeWhenForceIsEnabled)
{
	OutputPreviewParam param;
	EXPECT_TRUE(param.ShouldShowResultForExitCode(0));
	EXPECT_FALSE(param.ShouldShowResultForExitCode(1));

	param.mForce = 1;
	EXPECT_TRUE(param.ShouldShowResultForExitCode(1));
}

TEST(OutputPreviewCommandParamTest, MapsWindowDisplayTypes)
{
	OutputPreviewParam param;
	EXPECT_EQ(param.GetShowType(), SW_SHOWNORMAL);
	param.mShowType = 1;
	EXPECT_EQ(param.GetShowType(), SW_HIDE);
	param.mShowType = 2;
	EXPECT_EQ(param.GetShowType(), SW_SHOWMINIMIZED);
	param.mShowType = 3;
	EXPECT_EQ(param.GetShowType(), SW_SHOWMAXIMIZED);
}
