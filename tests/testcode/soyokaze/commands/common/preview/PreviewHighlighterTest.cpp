#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/common/preview/PreviewHighlighter.h"

using launcherapp::commands::common::preview::FindPreviewMatchRanges;

TEST(PreviewHighlighter, FindsEveryLiteralOccurrenceIgnoringAsciiCase)
{
	auto ranges = FindPreviewMatchRanges("Key key KEY", {"key"});

	ASSERT_EQ(3u, ranges.size());
	EXPECT_EQ(0u, ranges[0].mStart);
	EXPECT_EQ(3u, ranges[0].mLength);
	EXPECT_EQ(4u, ranges[1].mStart);
	EXPECT_EQ(8u, ranges[2].mStart);
}

TEST(PreviewHighlighter, PreservesUtf8ByteOffsets)
{
	const std::string text = "\xE3\x81\x82" "word" "\xE3\x81\x82";
	auto ranges = FindPreviewMatchRanges(text, {"word"});

	ASSERT_EQ(1u, ranges.size());
	EXPECT_EQ(3u, ranges[0].mStart);
	EXPECT_EQ(4u, ranges[0].mLength);
}

TEST(PreviewHighlighter, IgnoresEmptyKeywordsAndDeduplicatesRanges)
{
	auto ranges = FindPreviewMatchRanges("alpha", {"", "alpha", "alpha"});

	ASSERT_EQ(1u, ranges.size());
	EXPECT_EQ(0u, ranges[0].mStart);
	EXPECT_EQ(5u, ranges[0].mLength);
}
