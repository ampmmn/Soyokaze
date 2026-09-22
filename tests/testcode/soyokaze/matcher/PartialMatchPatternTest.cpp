#include "stdafx.h"
#include "gtest/gtest.h"
#include "matcher/PartialMatchPattern.h"

TEST(PartialMatchPattern, KeepsAbsolutePathWithSpacesAsOneToken)
{
	PartialMatchPattern* pattern = PartialMatchPattern::Create();
	pattern->SetWholeText(_T("mspaint.exe c:\\path with spaces\\"));

	std::vector<CString> words;
	pattern->GetRawWords(words);

	ASSERT_EQ(2u, words.size());
	EXPECT_EQ(_T("mspaint.exe"), words[0]);
	EXPECT_EQ(_T("c:\\path with spaces\\"), words[1]);

	pattern->Release();
}

TEST(PartialMatchPattern, SplitsWordsBySpaces)
{
	PartialMatchPattern* pattern = PartialMatchPattern::Create();
	pattern->SetWholeText(_T("mspaint.exe image.png"));

	std::vector<CString> words;
	pattern->GetRawWords(words);

	ASSERT_EQ(2u, words.size());
	EXPECT_EQ(_T("mspaint.exe"), words[0]);
	EXPECT_EQ(_T("image.png"), words[1]);

	pattern->Release();
}

TEST(PartialMatchPattern, SkipsConsecutiveSpaces)
{
	PartialMatchPattern* pattern = PartialMatchPattern::Create();
	pattern->SetWholeText(_T("mspaint.exe   image.png"));

	std::vector<CString> words;
	pattern->GetRawWords(words);

	ASSERT_EQ(2u, words.size());
	EXPECT_EQ(_T("mspaint.exe"), words[0]);
	EXPECT_EQ(_T("image.png"), words[1]);

	pattern->Release();
}

TEST(PartialMatchPattern, RemovesQuotesFromQuotedWord)
{
	PartialMatchPattern* pattern = PartialMatchPattern::Create();
	pattern->SetWholeText(_T("mspaint.exe \"image file.png\""));

	std::vector<CString> words;
	pattern->GetRawWords(words);

	ASSERT_EQ(2u, words.size());
	EXPECT_EQ(_T("mspaint.exe"), words[0]);
	EXPECT_EQ(_T("image file.png"), words[1]);

	pattern->Release();
}

TEST(PartialMatchPattern, KeepsAbsolutePathWithoutSpacesAsOneToken)
{
	PartialMatchPattern* pattern = PartialMatchPattern::Create();
	pattern->SetWholeText(_T("mspaint.exe c:\\image.png"));

	std::vector<CString> words;
	pattern->GetRawWords(words);

	ASSERT_EQ(2u, words.size());
	EXPECT_EQ(_T("mspaint.exe"), words[0]);
	EXPECT_EQ(_T("c:\\image.png"), words[1]);

	pattern->Release();
}

TEST(PartialMatchPattern, KeepsAbsolutePathWithoutCommandAsOneToken)
{
	PartialMatchPattern* pattern = PartialMatchPattern::Create();
	pattern->SetWholeText(_T("c:\\path with spaces\\"));

	std::vector<CString> words;
	pattern->GetRawWords(words);

	ASSERT_EQ(1u, words.size());
	EXPECT_EQ(_T("c:\\path with spaces\\"), words[0]);

	pattern->Release();
}

TEST(PartialMatchPattern, TrimsTrailingSpaces)
{
	PartialMatchPattern* pattern = PartialMatchPattern::Create();
	pattern->SetWholeText(_T("mspaint.exe image.png   "));

	std::vector<CString> words;
	pattern->GetRawWords(words);

	ASSERT_EQ(2u, words.size());
	EXPECT_EQ(_T("mspaint.exe"), words[0]);
	EXPECT_EQ(_T("image.png"), words[1]);

	pattern->Release();
}
