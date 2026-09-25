#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/UpdateInfo.h"

using namespace launcherapp::utility;

/**
  バージョン番号の解析と比較を検証する
*/
TEST(UpdateInfoTest, ParseAndCompareVersionNumber)
{
	VersionNumber parsed;
	ASSERT_TRUE(ParseVersionNumber(_T("0.60.12"), parsed));
	EXPECT_EQ(0U, parsed.mMajor);
	EXPECT_EQ(60U, parsed.mMinor);
	EXPECT_EQ(12U, parsed.mBuild);

	VersionNumber other;
	ASSERT_TRUE(ParseVersionNumber(_T("0.59.99"), other));
	EXPECT_TRUE(IsVersionNewer(parsed, other));
	EXPECT_FALSE(IsVersionNewer(other, parsed));
}

/**
  バージョン要素を桁数に依存せず順番に比較する
*/
TEST(UpdateInfoTest, CompareVersionComponentsIndependently)
{
	VersionNumber lower;
	VersionNumber higher;
	ASSERT_TRUE(ParseVersionNumber(_T("0.1.100"), lower));
	ASSERT_TRUE(ParseVersionNumber(_T("0.2.0"), higher));
	EXPECT_TRUE(IsVersionNewer(higher, lower));
	EXPECT_FALSE(IsVersionNewer(lower, higher));
}

/**
  不正なバージョン形式を拒否する
*/
TEST(UpdateInfoTest, RejectInvalidVersionNumber)
{
	VersionNumber parsed;
	EXPECT_FALSE(ParseVersionNumber(_T("0.60"), parsed));
	EXPECT_FALSE(ParseVersionNumber(_T("0..1"), parsed));
	EXPECT_FALSE(ParseVersionNumber(_T("0.60.1.2"), parsed));
	EXPECT_FALSE(ParseVersionNumber(_T("0.-1.2"), parsed));
	EXPECT_FALSE(ParseVersionNumber(_T("4294967296.0.0"), parsed));
}

/**
  更新JSONから必須項目と任意の日付を解析する
*/
TEST(UpdateInfoTest, ParseUpdateInfo)
{
	const std::string json = R"({"latest":{"version":"0.60.0","date":"2026-10-01","url":"https://github.com/ampmmn/Soyokaze/releases/latest"},"future":{"ignored":true}})";
	std::vector<BYTE> content(json.begin(), json.end());

	UpdateInfo info;
	ASSERT_TRUE(ParseUpdateInfo(content, info));
	EXPECT_EQ(_T("2026-10-01"), info.mDate);
	EXPECT_EQ(_T("https://github.com/ampmmn/Soyokaze/releases/latest"), info.mUrl);
	EXPECT_EQ(60U, info.mVersion.mMinor);
}

/**
  日付が省略された更新JSONを解析する
*/
TEST(UpdateInfoTest, ParseUpdateInfoWithoutDate)
{
	const std::string json = R"({"latest":{"version":"0.60.0","url":"https://github.com/ampmmn/Soyokaze/releases/latest"}})";
	std::vector<BYTE> content(json.begin(), json.end());

	UpdateInfo info;
	ASSERT_TRUE(ParseUpdateInfo(content, info));
	EXPECT_TRUE(info.mDate.IsEmpty());
}

/**
  必須項目や形式が不正な更新JSONを拒否する
*/
TEST(UpdateInfoTest, RejectInvalidUpdateInfo)
{
	const std::vector<std::string> invalidJsonList = {
		"invalid json",
		R"({"latest":{"date":"2026-10-01","url":"https://example.com"}})",
		R"({"latest":{"version":"0.60.0","url":"http://example.com"}})",
		R"({"latest":{"version":"0.60.0","date":"2026-02-30","url":"https://example.com"}})",
		R"({"latest":{"version":"0.60.0","date":123,"url":"https://example.com"}})"
	};

	for (const auto& json : invalidJsonList) {
		std::vector<BYTE> content(json.begin(), json.end());
		UpdateInfo info;
		EXPECT_FALSE(ParseUpdateInfo(content, info));
	}
}
