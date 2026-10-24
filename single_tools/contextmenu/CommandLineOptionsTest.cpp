#include "CommandLineOptions.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

/**
  テスト用の引数配列を作成する
  @param[in] arguments 引数の文字列一覧
  @return 引数の配列
*/
std::vector<LPWSTR> MakeArguments(std::vector<std::wstring> arguments)
{
	std::vector<LPWSTR> result;
	result.reserve(arguments.size());
	for (std::wstring& argument : arguments) {
		result.push_back(argument.data());
	}
	return result;
}

} // 名前空間

TEST(CommandLineOptionsTest, ParsesPaths)
{
	CommandLineOptions options;
	std::vector<std::wstring> arguments = { L"C:\\work\\a.txt", L"C:\\work\\b.txt" };
	const std::vector<LPWSTR> argv = MakeArguments(arguments);
	options.Parse(static_cast<int>(argv.size()), argv.data());

	ASSERT_EQ(options.GetPaths().size(), 2U);
	EXPECT_EQ(options.GetPaths()[0], std::wstring(L"C:\\work\\a.txt"));
	EXPECT_EQ(options.GetPaths()[1], std::wstring(L"C:\\work\\b.txt"));
	EXPECT_FALSE(options.HasNowait());
	EXPECT_FALSE(options.HasPosition());
}

TEST(CommandLineOptionsTest, ParsesNowait)
{
	CommandLineOptions options;
	std::vector<std::wstring> arguments = { L"--nowait", L"C:\\work\\a.txt" };
	const std::vector<LPWSTR> argv = MakeArguments(arguments);
	options.Parse(static_cast<int>(argv.size()), argv.data());

	EXPECT_TRUE(options.HasNowait());
	ASSERT_EQ(options.GetPaths().size(), 1U);
	EXPECT_EQ(options.GetPaths()[0], std::wstring(L"C:\\work\\a.txt"));
}

TEST(CommandLineOptionsTest, ClearsNowaitWhenParsingAgain)
{
	CommandLineOptions options;
	std::vector<std::wstring> firstArguments = { L"--nowait" };
	const std::vector<LPWSTR> firstArgv = MakeArguments(firstArguments);
	options.Parse(static_cast<int>(firstArgv.size()), firstArgv.data());

	std::vector<std::wstring> secondArguments = { L"C:\\work\\a.txt" };
	const std::vector<LPWSTR> secondArgv = MakeArguments(secondArguments);
	options.Parse(static_cast<int>(secondArgv.size()), secondArgv.data());

	EXPECT_FALSE(options.HasNowait());
}

TEST(CommandLineOptionsTest, ParsesPosition)
{
	CommandLineOptions options;
	std::vector<std::wstring> arguments = { L"--position=100,200", L"C:\\work\\a.txt" };
	const std::vector<LPWSTR> argv = MakeArguments(arguments);
	options.Parse(static_cast<int>(argv.size()), argv.data());

	EXPECT_TRUE(options.HasPosition());
	EXPECT_EQ(options.GetPosition().x, 100L);
	EXPECT_EQ(options.GetPosition().y, 200L);
	ASSERT_EQ(options.GetPaths().size(), 1U);
	EXPECT_EQ(options.GetPaths()[0], std::wstring(L"C:\\work\\a.txt"));
}

TEST(CommandLineOptionsTest, ParsesNegativePosition)
{
	CommandLineOptions options;
	std::vector<std::wstring> arguments = { L"--position=-100,-200" };
	const std::vector<LPWSTR> argv = MakeArguments(arguments);
	options.Parse(static_cast<int>(argv.size()), argv.data());

	EXPECT_TRUE(options.HasPosition());
	EXPECT_EQ(options.GetPosition().x, -100L);
	EXPECT_EQ(options.GetPosition().y, -200L);
	EXPECT_TRUE(options.GetPaths().empty());
}

TEST(CommandLineOptionsTest, IgnoresInvalidPosition)
{
	CommandLineOptions options;
	std::vector<std::wstring> arguments = { L"--position=abc", L"C:\\work\\a.txt" };
	const std::vector<LPWSTR> argv = MakeArguments(arguments);
	options.Parse(static_cast<int>(argv.size()), argv.data());

	// 不正な位置指定は位置未指定として扱い、パスの対象にはしない
	EXPECT_FALSE(options.HasPosition());
	ASSERT_EQ(options.GetPaths().size(), 1U);
	EXPECT_EQ(options.GetPaths()[0], std::wstring(L"C:\\work\\a.txt"));
}
