#include "stdafx.h"
#include "gtest/gtest.h"
#include "features/main/DescriptionCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TEST(DescriptionCtrl, KeepsOriginalSizeWhenWidthFits)
{
	EXPECT_EQ(100, DescriptionCtrl::SelectScalePercent({ 80 }, 100));
}

TEST(DescriptionCtrl, SelectsLargestFivePercentStepThatFits)
{
	EXPECT_EQ(90, DescriptionCtrl::SelectScalePercent({ 140, 130, 95 }, 100));
}

TEST(DescriptionCtrl, SelectsFiftyPercentWhenItIsTheFirstSizeThatFits)
{
	EXPECT_EQ(50, DescriptionCtrl::SelectScalePercent({ 140, 135, 130, 125, 120, 115, 110, 105, 102, 101, 98 }, 100));
}

TEST(DescriptionCtrl, StopsAtFiftyPercentWhenTextStillDoesNotFit)
{
	EXPECT_EQ(50, DescriptionCtrl::SelectScalePercent({ 140, 135, 130, 125, 120, 115, 110, 105, 102, 101, 100 }, 99));
}
