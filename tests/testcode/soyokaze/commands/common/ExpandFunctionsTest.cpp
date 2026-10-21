#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/common/ExpandFunctions.h"

using namespace launcherapp::commands::common;

TEST(ExpandFunctionsTest, ExpandArgumentsAlwaysQuotesValue)
{
	std::vector<CString> args;
	args.push_back(_T("foo"));

	CString target(_T("--file $1"));
	ExpandArguments(target, args);

	// スペースを含まない値も常にクォートされる
	EXPECT_STREQ(_T("--file \"foo\""), (LPCTSTR)target);
}

TEST(ExpandFunctionsTest, ExpandArgumentsQuotesValueContainingSpace)
{
	std::vector<CString> args;
	args.push_back(_T("foo bar"));

	CString target(_T("--file $1"));
	ExpandArguments(target, args);

	EXPECT_STREQ(_T("--file \"foo bar\""), (LPCTSTR)target);
}

TEST(ExpandFunctionsTest, ExpandArgumentsDoesNotQuoteWhenQuotingIsNone)
{
	std::vector<CString> args;
	args.push_back(_T("foo bar"));

	CString target(_T("--file $1"));
	ExpandArguments(target, args, ArgumentQuoting::None);

	EXPECT_STREQ(_T("--file foo bar"), (LPCTSTR)target);
}

TEST(ExpandFunctionsTest, ExpandArgumentsDoesNotDoubleQuoteWhenTemplateIsQuoted)
{
	std::vector<CString> args;
	args.push_back(_T("foo bar"));

	// テンプレート側で引用符が付いている場合は、二重に引用符を付さない
	CString target(_T("--file \"$1\""));
	ExpandArguments(target, args);

	EXPECT_STREQ(_T("--file \"foo bar\""), (LPCTSTR)target);
}

TEST(ExpandFunctionsTest, ExpandArgumentsSupportsArgumentNumberGreaterThanNine)
{
	std::vector<CString> args;
	for (int i = 0; i < 10; ++i) {
		CString arg;
		arg.Format(_T("arg%d"), i + 1);
		args.push_back(arg);
	}

	CString target(_T("$10"));
	ExpandArguments(target, args);

	EXPECT_STREQ(_T("\"arg10\""), (LPCTSTR)target);
}

TEST(ExpandFunctionsTest, ExpandArgumentsFallsBackToShorterArgumentNumberWhenOutOfRange)
{
	std::vector<CString> args;
	args.push_back(_T("arg1"));

	// 引数の数を超える場合は、1桁少ない引数番号として扱う(従来の挙動を維持する)
	CString target(_T("$12"));
	ExpandArguments(target, args);

	EXPECT_STREQ(_T("\"arg1\"2"), (LPCTSTR)target);
}

TEST(ExpandFunctionsTest, ExpandArgumentsKeepsOutOfRangeArgumentNumber)
{
	std::vector<CString> args;
	args.push_back(_T("arg1"));

	CString target(_T("--file $2"));
	ExpandArguments(target, args);

	EXPECT_STREQ(_T("--file $2"), (LPCTSTR)target);
}

TEST(ExpandFunctionsTest, ExpandArgumentsExpandsAllArguments)
{
	std::vector<CString> args;
	args.push_back(_T("a"));
	args.push_back(_T("b c"));

	CString target(_T("$*"));
	ExpandArguments(target, args);

	EXPECT_STREQ(_T("\"a\" \"b c\""), (LPCTSTR)target);
}

TEST(ExpandFunctionsTest, ExpandArgumentsDoesNotReinterpretExpandedValue)
{
	// 展開された値に含まれる$nが再度解釈されることはない
	std::vector<CString> args;
	args.push_back(_T("$2"));
	args.push_back(_T("second"));

	CString target(_T("--file $1"));
	ExpandArguments(target, args);

	EXPECT_STREQ(_T("--file \"$2\""), (LPCTSTR)target);
}

TEST(ExpandFunctionsTest, ExpandArgumentsEscapesQuoteInValue)
{
	// 値に含まれるダブルクォートはエスケープされる
	std::vector<CString> args;
	args.push_back(_T("a\"b"));

	CString target(_T("--file $1"));
	ExpandArguments(target, args);

	EXPECT_STREQ(_T("--file \"a\"\"b\""), (LPCTSTR)target);
}

TEST(ExpandFunctionsTest, ExpandArgumentsAndMacrosDoesNotReinterpretExpandedValue)
{
	// マクロの展開結果に含まれる$nが解釈されることはない
	std::vector<CString> args;
	args.push_back(_T("$2"));
	args.push_back(_T("second"));

	CString target(_T("--file $1"));
	ExpandArgumentsAndMacros(target, args, ArgumentQuoting::Always, true);

	EXPECT_STREQ(_T("--file \"$2\""), (LPCTSTR)target);
}