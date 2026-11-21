#include "SearchViewerWindowPlacement.h"

#include <gtest/gtest.h>

#include <string>

TEST(SearchViewerWindowPlacementTest, EncodesAndDecodesPlacement)
{
	WINDOWPLACEMENT original{};
	original.length = sizeof(original);
	original.showCmd = SW_SHOWNORMAL;
	original.rcNormalPosition = RECT{ 10, 20, 1010, 720 };

	const std::string encoded = SearchViewerWindowPlacement::Encode(original);
	ASSERT_FALSE(encoded.empty());
	WINDOWPLACEMENT restored{};
	ASSERT_TRUE(SearchViewerWindowPlacement::Decode(encoded, restored));
	EXPECT_EQ(restored.length, sizeof(WINDOWPLACEMENT));
	EXPECT_EQ(restored.rcNormalPosition.left, 10L);
	EXPECT_EQ(restored.rcNormalPosition.bottom, 720L);
}

TEST(SearchViewerWindowPlacementTest, RejectsInvalidBase64)
{
	WINDOWPLACEMENT placement{};
	EXPECT_FALSE(SearchViewerWindowPlacement::Decode("not base64", placement));
}

TEST(SearchViewerWindowPlacementTest, UpdatesAndReadsProfilePlacement)
{
	WINDOWPLACEMENT original{};
	original.length = sizeof(original);
	original.showCmd = SW_SHOWNORMAL;
	original.rcNormalPosition = RECT{ 40, 50, 840, 650 };

	const std::string yaml = SearchViewerWindowPlacement::Update({}, L"google", original);
	ASSERT_FALSE(yaml.empty());
	WINDOWPLACEMENT restored{};
	ASSERT_TRUE(SearchViewerWindowPlacement::Parse(yaml, L"google", restored));
	EXPECT_EQ(restored.rcNormalPosition.left, 40L);
	EXPECT_EQ(restored.rcNormalPosition.top, 50L);
}

TEST(SearchViewerWindowPlacementTest, PreservesOtherProfilesWhenUpdating)
{
	WINDOWPLACEMENT first{};
	first.length = sizeof(first);
	first.showCmd = SW_SHOWNORMAL;
	first.rcNormalPosition = RECT{ 10, 20, 610, 420 };
	std::string yaml = SearchViewerWindowPlacement::Update({}, L"google", first);
	ASSERT_FALSE(yaml.empty());

	WINDOWPLACEMENT second{};
	second.length = sizeof(second);
	second.showCmd = SW_SHOWMAXIMIZED;
	second.rcNormalPosition = RECT{ 100, 120, 900, 720 };
	yaml = SearchViewerWindowPlacement::Update(yaml, L"bing", second);
	ASSERT_FALSE(yaml.empty());

	WINDOWPLACEMENT restoredFirst{};
	WINDOWPLACEMENT restoredSecond{};
	ASSERT_TRUE(SearchViewerWindowPlacement::Parse(yaml, L"google", restoredFirst));
	ASSERT_TRUE(SearchViewerWindowPlacement::Parse(yaml, L"bing", restoredSecond));
	EXPECT_EQ(restoredFirst.rcNormalPosition.left, 10L);
	EXPECT_EQ(restoredSecond.showCmd, SW_SHOWMAXIMIZED);
}
