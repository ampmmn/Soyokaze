#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/decodestring/EscapedCharCommand.h"

using EscapedCharCommand = launcherapp::commands::decodestring::EscapedCharCommand;

TEST(EscapedCharCommand, ScanAsU4_1)
{
	std::string s("\\u0001");

	auto it = s.begin();
	std::string out;
	EXPECT_TRUE(EscapedCharCommand::ScanAsU4(it, s.end(), out));

	EXPECT_TRUE(it == s.end());

	EXPECT_EQ(1, out.size());
	EXPECT_EQ((char)1, out[0]);
}

TEST(EscapedCharCommand, ScanAsU4_2)
{
	std::string s("\\u0002\\u0053");

	auto it = s.begin();
	std::string out;
	EXPECT_TRUE(EscapedCharCommand::ScanAsU4(it, s.end(), out));

	EXPECT_EQ(6, std::distance(s.begin(), it));

	EXPECT_EQ(1, out.size());
	EXPECT_EQ((char)2, out[0]);

	EXPECT_TRUE(EscapedCharCommand::ScanAsU4(it, s.end(), out));

	EXPECT_TRUE(it == s.end());

	ASSERT_EQ(2, out.size());
	EXPECT_EQ('S', out[1]);
}

TEST(EscapedCharCommand, ScanAsU4_3)
{
	std::string s("\\0001");

	auto it = s.begin();
	std::string out;
	EXPECT_FALSE(EscapedCharCommand::ScanAsU4(it, s.end(), out));
}

TEST(EscapedCharCommand, ScanAsU4_4)
{
	std::string s("\\uHHHH");

	auto it = s.begin();
	std::string out;
	EXPECT_FALSE(EscapedCharCommand::ScanAsU4(it, s.end(), out));
}

TEST(EscapedCharCommand, ScanAsU4_5)
{
	std::string s("\\uD840\\uDC0B");

	auto it = s.begin();
	std::string out;
	EXPECT_TRUE(EscapedCharCommand::ScanAsU4(it, s.end(), out));

	EXPECT_TRUE(it == s.end());

	EXPECT_EQ(4, out.size());
	EXPECT_EQ(0xf0, (uint8_t)out[0]);
	EXPECT_EQ(0xa0, (uint8_t)out[1]);
	EXPECT_EQ(0x80, (uint8_t)out[2]);
	EXPECT_EQ(0x8b, (uint8_t)out[3]);
}

TEST(EscapedCharCommand, ScanAsU8_1)
{
	std::string s("\\U00000001");

	auto it = s.begin();
	std::string out;
	EXPECT_TRUE(EscapedCharCommand::ScanAsU8(it, s.end(), out));

	EXPECT_TRUE(it == s.end());

	EXPECT_EQ(1, out.size());
	EXPECT_EQ((char)1, out[0]);
}

TEST(EscapedCharCommand, ScanAsU8_2)
{
	std::string s("\\U00000002\\U00000053");

	auto it = s.begin();
	std::string out;
	EXPECT_TRUE(EscapedCharCommand::ScanAsU8(it, s.end(), out));

	EXPECT_EQ(10, std::distance(s.begin(), it));

	EXPECT_EQ(1, out.size());
	EXPECT_EQ((char)2, out[0]);

	EXPECT_TRUE(EscapedCharCommand::ScanAsU8(it, s.end(), out));

	EXPECT_TRUE(it == s.end());

	ASSERT_EQ(2, out.size());
	EXPECT_EQ('S', out[1]);
}

TEST(EscapedCharCommand, ScanAsU8_3)
{
	std::string s("\\0001");

	auto it = s.begin();
	std::string out;
	EXPECT_FALSE(EscapedCharCommand::ScanAsU8(it, s.end(), out));
}

TEST(EscapedCharCommand, ScanAsU8_4)
{
	std::string s("\\UH0000000");

	auto it = s.begin();
	std::string out;
	EXPECT_FALSE(EscapedCharCommand::ScanAsU8(it, s.end(), out));
}

TEST(EscapedCharCommand, ScanAsU8_5)
{
	std::string s("\\U0002000B");

	auto it = s.begin();
	std::string out;
	EXPECT_TRUE(EscapedCharCommand::ScanAsU8(it, s.end(), out));

	EXPECT_TRUE(it == s.end());

	EXPECT_EQ(4, out.size());
	EXPECT_EQ(0xf0, (uint8_t)out[0]);
	EXPECT_EQ(0xa0, (uint8_t)out[1]);
	EXPECT_EQ(0x80, (uint8_t)out[2]);
	EXPECT_EQ(0x8b, (uint8_t)out[3]);
}

TEST(EscapedCharCommand, ScanAsHex_1)
{
	std::string s("\\x01");

	auto it = s.begin();
	std::string out;
	EXPECT_TRUE(EscapedCharCommand::ScanAsHex(it, s.end(), out));

	EXPECT_TRUE(it == s.end());

	EXPECT_EQ(1, out.size());
	EXPECT_EQ((char)1, out[0]);
}

TEST(EscapedCharCommand, ScanAsHex_2)
{
	std::string s("\\x02\\x53");

	auto it = s.begin();
	std::string out;
	EXPECT_TRUE(EscapedCharCommand::ScanAsHex(it, s.end(), out));

	EXPECT_EQ(4, std::distance(s.begin(), it));

	EXPECT_EQ(1, out.size());
	EXPECT_EQ((char)2, out[0]);

	EXPECT_TRUE(EscapedCharCommand::ScanAsHex(it, s.end(), out));

	EXPECT_TRUE(it == s.end());

	ASSERT_EQ(2, out.size());
	EXPECT_EQ('S', out[1]);
}

TEST(EscapedCharCommand, ScanAsHex_3)
{
	std::string s("\\x1");

	auto it = s.begin();
	std::string out;
	EXPECT_FALSE(EscapedCharCommand::ScanAsHex(it, s.end(), out));
}

TEST(EscapedCharCommand, ScanAsHex_4)
{
	std::string s("\\xH0");

	auto it = s.begin();
	std::string out;
	EXPECT_FALSE(EscapedCharCommand::ScanAsHex(it, s.end(), out));
}

TEST(EscapedCharCommand, ScanAsOctal_1)
{
	std::string s("\\001");

	auto it = s.begin();
	std::string out;
	EXPECT_TRUE(EscapedCharCommand::ScanAsOctal(it, s.end(), out));

	EXPECT_TRUE(it == s.end());

	EXPECT_EQ(1, out.size());
	EXPECT_EQ((char)1, out[0]);
}

TEST(EscapedCharCommand, ScanAsOctal_2)
{
	std::string s("\\002\\123");

	auto it = s.begin();
	std::string out;
	EXPECT_TRUE(EscapedCharCommand::ScanAsOctal(it, s.end(), out));

	EXPECT_EQ(4, std::distance(s.begin(), it));

	EXPECT_EQ(1, out.size());
	EXPECT_EQ((char)2, out[0]);

	EXPECT_TRUE(EscapedCharCommand::ScanAsOctal(it, s.end(), out));

	EXPECT_TRUE(it == s.end());

	ASSERT_EQ(2, out.size());
	EXPECT_EQ('S', out[1]);
}

TEST(EscapedCharCommand, ScanAsOctal_3)
{
	std::string s("\\01");

	auto it = s.begin();
	std::string out;
	EXPECT_FALSE(EscapedCharCommand::ScanAsOctal(it, s.end(), out));
}

TEST(EscapedCharCommand, ScanAsOctal_4)
{
	std::string s("\\x0H0");

	auto it = s.begin();
	std::string out;
	EXPECT_FALSE(EscapedCharCommand::ScanAsOctal(it, s.end(), out));
}

