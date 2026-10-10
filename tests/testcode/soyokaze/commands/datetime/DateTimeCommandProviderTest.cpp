#include "stdafx.h"
#include "commands/datetime/DateTimeCommandProvider.h"

using launcherapp::commands::datetime::DateTimeCommandProvider;
using launcherapp::commands::datetime::DateTimeKind;

// 時間差の入力(HH:MM-HH:MM)が解析できること
TEST(DateTimeCommandProviderTest, ParseTimeSpanAcceptsValidInput)
{
	CTimeSpan result;
	EXPECT_TRUE(DateTimeCommandProvider::ParseTimeSpan(_T("13:43-12:50"), result));
	EXPECT_EQ(53, (int)result.GetTotalMinutes());
}

// 形式が不正、または24時以上の入力は解析されないこと
TEST(DateTimeCommandProviderTest, ParseTimeSpanRejectsInvalidInput)
{
	CTimeSpan result;
	EXPECT_FALSE(DateTimeCommandProvider::ParseTimeSpan(_T("13:43"), result));
	EXPECT_FALSE(DateTimeCommandProvider::ParseTimeSpan(_T("25:00-12:00"), result));
	EXPECT_FALSE(DateTimeCommandProvider::ParseTimeSpan(_T("12:00-25:00"), result));
	EXPECT_FALSE(DateTimeCommandProvider::ParseTimeSpan(_T("abc"), result));
}

// 英語形式の日数オフセット(later は正、ago は負)が解析できること
TEST(DateTimeCommandProviderTest, ParseDayOffsetAcceptsEnglishForm)
{
	int days = 0;
	EXPECT_TRUE(DateTimeCommandProvider::ParseDayOffset(_T("3 days later"), days));
	EXPECT_EQ(3, days);
	EXPECT_TRUE(DateTimeCommandProvider::ParseDayOffset(_T("1 day ago"), days));
	EXPECT_EQ(-1, days);
}

// 日本語形式の日数オフセット(後は正、前は負)が解析できること
TEST(DateTimeCommandProviderTest, ParseDayOffsetAcceptsJapaneseForm)
{
	int days = 0;
	EXPECT_TRUE(DateTimeCommandProvider::ParseDayOffset(_T("5日後"), days));
	EXPECT_EQ(5, days);
	EXPECT_TRUE(DateTimeCommandProvider::ParseDayOffset(_T("2日前"), days));
	EXPECT_EQ(-2, days);
}

// 日数オフセットの形式に一致しない入力は解析されないこと
TEST(DateTimeCommandProviderTest, ParseDayOffsetRejectsInvalidInput)
{
	int days = 0;
	EXPECT_FALSE(DateTimeCommandProvider::ParseDayOffset(_T("later"), days));
	EXPECT_FALSE(DateTimeCommandProvider::ParseDayOffset(_T("3 weeks later"), days));
	EXPECT_FALSE(DateTimeCommandProvider::ParseDayOffset(_T("1 + 1"), days));
}

// 上限を超える日数オフセットは解析されないこと
TEST(DateTimeCommandProviderTest, ParseDayOffsetRejectsOverLimit)
{
	int days = 0;
	EXPECT_TRUE(DateTimeCommandProvider::ParseDayOffset(_T("36500 days later"), days));
	EXPECT_EQ(36500, days);
	EXPECT_FALSE(DateTimeCommandProvider::ParseDayOffset(_T("36501 days later"), days));
	EXPECT_FALSE(DateTimeCommandProvider::ParseDayOffset(_T("99999日前"), days));
}

// 基準日時から日時候補が4件(日時/日付/時刻/曜日)の順で作られること
TEST(DateTimeCommandProviderTest, MakeDateTimeCandidatesReturnsFourItems)
{
	CTime base(2026, 10, 10, 14, 17, 0);
	auto candidates = DateTimeCommandProvider::MakeDateTimeCandidates(base);
	ASSERT_EQ(4, (int)candidates.size());
	EXPECT_STREQ(_T("2026/10/10 14:17"), candidates[0].first);
	EXPECT_STREQ(_T("日時"), candidates[0].second);
	EXPECT_STREQ(_T("2026/10/10"), candidates[1].first);
	EXPECT_STREQ(_T("日付"), candidates[1].second);
	EXPECT_STREQ(_T("14:17"), candidates[2].first);
	EXPECT_STREQ(_T("時刻"), candidates[2].second);
	EXPECT_STREQ(_T("土曜日"), candidates[3].first);
	EXPECT_STREQ(_T("曜日"), candidates[3].second);
}

// 日数オフセットを加えた基準日時から候補を作ると日付がずれること(2026/10/10 + 3日 = 2026/10/13 火曜日)
TEST(DateTimeCommandProviderTest, MakeDateTimeCandidatesAppliesOffset)
{
	CTime base = CTime(2026, 10, 10, 14, 17, 0) + CTimeSpan(3, 0, 0, 0);
	auto candidates = DateTimeCommandProvider::MakeDateTimeCandidates(base);
	ASSERT_EQ(4, (int)candidates.size());
	EXPECT_STREQ(_T("2026/10/13 14:17"), candidates[0].first);
	EXPECT_STREQ(_T("2026/10/13"), candidates[1].first);
	EXPECT_STREQ(_T("火曜日"), candidates[3].first);
}

// 日時を各種別の文字列に変換できること(2026/10/10 は土曜日)
TEST(DateTimeCommandProviderTest, FormatDateTimeReturnsExpectedStrings)
{
	CTime t(2026, 10, 10, 14, 17, 0);
	EXPECT_STREQ(_T("2026/10/10 14:17"), (LPCTSTR)DateTimeCommandProvider::FormatDateTime(t, DateTimeKind::DateTime));
	EXPECT_STREQ(_T("2026/10/10"), (LPCTSTR)DateTimeCommandProvider::FormatDateTime(t, DateTimeKind::Date));
	EXPECT_STREQ(_T("14:17"), (LPCTSTR)DateTimeCommandProvider::FormatDateTime(t, DateTimeKind::Time));
	EXPECT_STREQ(_T("土曜日"), (LPCTSTR)DateTimeCommandProvider::FormatDateTime(t, DateTimeKind::Weekday));
}
