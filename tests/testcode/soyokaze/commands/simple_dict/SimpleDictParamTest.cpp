#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include "commands/transfer/CommandJSONEntry.h"

using SimpleDictParam = launcherapp::commands::simple_dict::SimpleDictParam;
using launcherapp::commands::transfer::CommandJSONEntry;

TEST(SimpleDictParamTest, PreviewIsDisabledByDefault)
{
	SimpleDictParam param;

	EXPECT_FALSE(param.mIsUsePreview);
}

TEST(SimpleDictParamTest, SaveAndLoadPreviewSetting)
{
	SimpleDictParam param;
	param.mName = _T("dictionary");
	param.mIsUsePreview = TRUE;

	CommandJSONEntry entry(_T("dictionary"));
	entry.Init();
	ASSERT_TRUE(param.Save(&entry));

	SimpleDictParam loaded;
	ASSERT_TRUE(loaded.Load(&entry));
	EXPECT_TRUE(loaded.mIsUsePreview);

	param.mIsUsePreview = FALSE;
	ASSERT_TRUE(param.Save(&entry));
	ASSERT_TRUE(loaded.Load(&entry));
	EXPECT_FALSE(loaded.mIsUsePreview);
}

TEST(SimpleDictParamTest, LegacyEntryDisablesPreview)
{
	CommandJSONEntry entry(_T("dictionary"));
	entry.Init();

	SimpleDictParam loaded;
	ASSERT_TRUE(loaded.Load(&entry));
	EXPECT_FALSE(loaded.mIsUsePreview);
}

TEST(SimpleDictParamTest, PreviewSettingParticipatesInComparisonAndSwap)
{
	SimpleDictParam first;
	SimpleDictParam second;
	second.mIsUsePreview = TRUE;
	EXPECT_FALSE(first == second);

	SimpleDictParam copied(second);
	EXPECT_TRUE(copied.mIsUsePreview);

	first.swap(second);
	EXPECT_TRUE(first.mIsUsePreview);
	EXPECT_FALSE(second.mIsUsePreview);
	EXPECT_FALSE(first == second);
}
