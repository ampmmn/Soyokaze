#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/CommandLineArgument.h"

using launcherapp::utility::ContainsExpandableEnvReference;
using launcherapp::utility::EscapeArgumentValue;
using launcherapp::utility::QuoteArgument;
using launcherapp::utility::ReplaceQuotedPlaceholder;

namespace {

// CommandLineToArgvWで解析して、引数に分解する
// (クォートした文字列が正しく解析されることを確認するために使用する)
// なお、argv[0]はプログラム名として特殊な解釈されるため、
// 先頭にダミーのプログラム名を挿入して解析する
std::vector<CString> ParseCommandLine(const CString& commandLine)
{
	std::vector<CString> args;

	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(commandLine, &argc);
	if (argv == nullptr) {
		return args;
	}

	for (int i = 0; i < argc; ++i) {
		args.push_back(CString(argv[i]));
	}
	// CommandLineToArgvWが返した領域はまとめて解放する
	GlobalFree(argv);

	return args;
}

// 値だけを解析した結果を取得する
std::vector<CString> ParseArgument(const CString& value)
{
	CString commandLine(_T("dummy "));
	commandLine += value;

	auto args = ParseCommandLine(commandLine);

	// ダミーのプログラム名を除く
	if (args.empty() == false) {
		args.erase(args.begin());
	}
	return args;
}

} // namespace

TEST(CommandLineArgumentTest, QuoteArgumentAlwaysQuotesValue)
{
	// スペースを含まない値も常にクォートされる
	EXPECT_STREQ(_T("\"foo\""), (LPCTSTR)QuoteArgument(_T("foo")));
	EXPECT_STREQ(_T("\"foo bar\""), (LPCTSTR)QuoteArgument(_T("foo bar")));
	EXPECT_STREQ(_T("\"\""), (LPCTSTR)QuoteArgument(_T("")));
}

TEST(CommandLineArgumentTest, QuoteArgumentEscapesEmbeddedQuote)
{
	// 埋め込みのダブルクォートは""になる
	EXPECT_STREQ(_T("\"a\"\"b\""), (LPCTSTR)QuoteArgument(_T("a\"b")));
	EXPECT_STREQ(_T("\"\"\"\""), (LPCTSTR)QuoteArgument(_T("\"")));
	EXPECT_STREQ(_T("\"a\"\"\"\"b\""), (LPCTSTR)QuoteArgument(_T("a\"\"b")));
}

TEST(CommandLineArgumentTest, QuoteArgumentEscapesBackslashBeforeQuote)
{
	// ダブルクォートに続くバックスラッシュは2倍になる
	EXPECT_STREQ(_T("\"a\\\\\"\"b\""), (LPCTSTR)QuoteArgument(_T("a\\\"b")));

	// 末尾のバックスラッシュも2倍になる(閉じ引用符をエスケープさせないため)
	EXPECT_STREQ(_T("\"C:\\path\\\\\""), (LPCTSTR)QuoteArgument(_T("C:\\path\\")));

	// ダブルクォートに続かないバックスラッシュはそのまま
	EXPECT_STREQ(_T("\"C:\\path\""), (LPCTSTR)QuoteArgument(_T("C:\\path")));
}

TEST(CommandLineArgumentTest, QuotedValueIsRestoredByCommandLineToArgvW)
{
	// クォートした値がそのまま復元されることを確認する
	std::vector<CString> values = {
		_T("foo"),
		_T("foo bar"),
		_T("a\"b"),
		_T("a\\\"b"),
		_T("C:\\path\\"),
		_T("C:\\path with space\\file.txt"),
		_T("a&b"),
		_T("a|b>c<d"),
		_T("$(whoami)"),
		_T("a`b"),
	};

	for (auto& value : values) {
		CString quoted = QuoteArgument(value);

		auto args = ParseArgument(quoted);

		ASSERT_EQ(1, (int)args.size());
		EXPECT_STREQ((LPCTSTR)value, (LPCTSTR)args[0]);
	}
}

TEST(CommandLineArgumentTest, EscapedValueIsRestoredByCommandLineToArgvW)
{
	// 外側の引用符を付けない形式でも正しく復元されることを確認する
	std::vector<CString> values = {
		_T("foo"),
		_T("a\"b"),
		_T("C:\\path\\"),
	};

	for (auto& value : values) {
		CString escaped = _T("\"") + EscapeArgumentValue(value) + _T("\"");

		auto args = ParseArgument(escaped);

		ASSERT_EQ(1, (int)args.size());
		EXPECT_STREQ((LPCTSTR)value, (LPCTSTR)args[0]);
	}
}

TEST(CommandLineArgumentTest, ContainsExpandableEnvReferenceDetectsDefinedVariable)
{
	// 定義済みの環境変数を参照している場合はtrueになる
	EXPECT_TRUE(ContainsExpandableEnvReference(_T("%PATH%")));

	// 定義されていない環境変数の場合はfalseになる
	EXPECT_FALSE(ContainsExpandableEnvReference(_T("%SOYOKAZE_TEST_UNDEFINED_VARIABLE%")));

	// 環境変数を参照していない場合はfalseになる
	EXPECT_FALSE(ContainsExpandableEnvReference(_T("foo")));
	EXPECT_FALSE(ContainsExpandableEnvReference(_T("50%")));
	EXPECT_FALSE(ContainsExpandableEnvReference(_T("100% off")));
	EXPECT_FALSE(ContainsExpandableEnvReference(_T("")));
}

TEST(CommandLineArgumentTest, ReplaceQuotedPlaceholderQuotesValue)
{
	CString text(_T("--target=$target"));

	ReplaceQuotedPlaceholder(text, _T("$target"), _T("http://example.com/"));

	EXPECT_STREQ(_T("--target=\"http://example.com/\""), (LPCTSTR)text);
}

TEST(CommandLineArgumentTest, ReplaceQuotedPlaceholderDoesNotDoubleQuoteWhenAlreadyQuoted)
{
	CString text(_T("--target=\"$target\""));

	ReplaceQuotedPlaceholder(text, _T("$target"), _T("http://example.com/"));

	EXPECT_STREQ(_T("--target=\"http://example.com/\""), (LPCTSTR)text);
}