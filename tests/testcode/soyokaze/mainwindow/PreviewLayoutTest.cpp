#include "stdafx.h"
#include "gtest/gtest.h"
#include "features/main/preview/PreviewLayout.h"

using launcherapp::mainwindow::preview::CalculatePreviewLayout;

TEST(PreviewLayout, HidesPreviewWhenClientWidthIsBelowMinimum)
{
	auto layout = CalculatePreviewLayout(639, 2, 2);

	EXPECT_FALSE(layout.mIsPreviewVisible);
	EXPECT_EQ(635, layout.mCandidateWidth);
	EXPECT_EQ(0, layout.mPreviewWidth);
}

TEST(PreviewLayout, HidesPreviewAtMinimumCandidateWidth)
{
	auto layout = CalculatePreviewLayout(640, 2, 2);

	EXPECT_FALSE(layout.mIsPreviewVisible);
	EXPECT_EQ(636, layout.mCandidateWidth);
}

TEST(PreviewLayout, KeepsOuterMarginWhenPreviewIsTooNarrow)
{
	auto layout = CalculatePreviewLayout(641, 2, 2);

	EXPECT_FALSE(layout.mIsPreviewVisible);
	EXPECT_EQ(637, layout.mCandidateWidth);
}

TEST(PreviewLayout, UsesFixedCandidatePaneUntilWideLayout)
{
	auto layout = CalculatePreviewLayout(1279, 2, 2);

	EXPECT_TRUE(layout.mIsPreviewVisible);
	EXPECT_EQ(638, layout.mCandidateWidth);
	EXPECT_EQ(640, layout.mPreviewX);
	EXPECT_EQ(637, layout.mPreviewWidth);
}

TEST(PreviewLayout, SplitsWideLayoutInHalf)
{
	auto layout = CalculatePreviewLayout(1280, 2, 2);

	EXPECT_TRUE(layout.mIsPreviewVisible);
	EXPECT_EQ(638, layout.mCandidateWidth);
	EXPECT_EQ(640, layout.mPreviewX);
	EXPECT_EQ(638, layout.mPreviewWidth);
}
