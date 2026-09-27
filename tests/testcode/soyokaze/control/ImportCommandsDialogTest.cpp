#include "stdafx.h"
#include "gtest/gtest.h"
#include "features/keywordmanager/ImportCommandsDialog.h"

TEST(ImportCommandsDialogTest, ImportIsDisabledWhenNothingIsSelected)
{
	EXPECT_FALSE(ImportCommandsDialog::CanImport(0));
}

TEST(ImportCommandsDialogTest, ImportIsEnabledWhenAtLeastOneItemIsSelected)
{
	EXPECT_TRUE(ImportCommandsDialog::CanImport(1));
	EXPECT_TRUE(ImportCommandsDialog::CanImport(3));
}
