#include "stdafx.h"
#include "gtest/gtest.h"
#include "matcher/CommandToken.h"
#include "utility/Path.h"

using CommandToken = launcherapp::matcher::CommandToken;

TEST(CommandToken, constructor)
{
	CommandToken tok(_T("a b"));
	EXPECT_EQ(2, tok.GetCount());
}

TEST(CommandToken, destructor)
{
	CommandToken tok(_T("a b"));
}

TEST(CommandToken, TestGetCount)
{
	CommandToken tok(_T("a b"));
	EXPECT_EQ(2, tok.GetCount());
}

TEST(CommandToken, EmptyAndWhitespaceOnlyInputsHaveNoTokens)
{
	CommandToken empty(_T(""));
	CommandToken spaces(_T("   "));

	EXPECT_EQ(0u, empty.GetCount());
	EXPECT_EQ(0u, spaces.GetCount());
}

TEST(CommandToken, SkipsLeadingRepeatedAndTrailingSpaces)
{
	CommandToken tok(_T("  alpha   \"beta gamma\"  "));

	ASSERT_EQ(2u, tok.GetCount());
	CString token;
	ASSERT_TRUE(tok.GetToken(0, token));
	EXPECT_EQ(_T("alpha"), token);
	ASSERT_TRUE(tok.GetToken(1, token));
	EXPECT_EQ(_T("\"beta gamma\""), token);
}

TEST(CommandToken, GetTrailingString1)
{
	CommandToken tok(_T("a b"));

	CString text;
	bool result = tok.GetTrailingString(1, text);
	EXPECT_TRUE(result);
	EXPECT_EQ(_T("b"), text);
}

TEST(CommandToken, GetTrailingString2)
{
	CommandToken tok(_T("a b"));

	CString text;
	bool result = tok.GetTrailingString(0, text);
	EXPECT_TRUE(result);
	EXPECT_EQ(_T("a b"), text);
}

TEST(CommandToken, GetTrailingString3)
{
	CommandToken tok(_T("a b"));

	CString text;
	bool result = tok.GetTrailingString(3, text);
	EXPECT_FALSE(result);
}

TEST(CommandToken, GetTrailingStringPreservesQuotedRemainder)
{
	CommandToken tok(_T("alpha \"beta gamma\" tail"));

	CString text;
	ASSERT_TRUE(tok.GetTrailingString(6, text));
	EXPECT_EQ(_T("\"beta gamma\" tail"), text);
}

TEST(CommandToken, GetTokenRange)
{
	CommandToken tok(_T("hoge param tail"));

	int start = 0;
	int end = 0;
	EXPECT_TRUE(tok.GetTokenRange(7, start, end));
	EXPECT_EQ(5, start);
	EXPECT_EQ(10, end);
}

TEST(CommandToken, GetTokenRangeLeavesRepeatedSpaceGapUnassigned)
{
	CommandToken tok(_T("alpha   beta  "));

	int start = 0;
	int end = 0;
	ASSERT_TRUE(tok.GetTokenRange(0, start, end));
	EXPECT_EQ(0, start);
	EXPECT_EQ(5, end);
	EXPECT_FALSE(tok.GetTokenRange(6, start, end));
	ASSERT_TRUE(tok.GetTokenRange(8, start, end));
	EXPECT_EQ(8, start);
	EXPECT_EQ(12, end);
	ASSERT_TRUE(tok.GetTokenRange(12, start, end));
	EXPECT_FALSE(tok.GetTokenRange(13, start, end));
}

TEST(CommandToken, GetToken)
{
	CommandToken tok(_T("hoge \"param value\" tail"));

	CString token;
	EXPECT_TRUE(tok.GetToken(1, token));
	EXPECT_EQ(_T("\"param value\""), token);
	EXPECT_FALSE(tok.GetToken(3, token));
}

TEST(CommandToken, GetParameterRangeAfterAbsoluteExecutable)
{
	Path executablePath(Path::MODULEFILEPATH);
	CString executable((LPCTSTR)executablePath);
	CString input = executable + _T(" C:\\");
	CommandToken tok(input);

	int start = 0;
	int end = 0;
	if (tok.GetPathParameterRange(start, end) == false) {
		ASSERT_TRUE(tok.GetTokenRange(input.GetLength(), start, end));
	}
	EXPECT_EQ(_T("C:\\"), input.Mid(start, end - start));
}

TEST(CommandToken, GetsAbsolutePathParameterWithSpacesAfterAbsoluteExecutable)
{
	Path executablePath(Path::MODULEFILEPATH);
	CString executable((LPCTSTR)executablePath);
	CString input = executable + _T(" C:\\folder with spaces\\");
	CommandToken tok(input);

	int start = 0;
	int end = 0;
	ASSERT_TRUE(tok.GetPathParameterRange(start, end));
	EXPECT_EQ(_T("C:\\folder with spaces\\"), input.Mid(start, end - start));
}

TEST(CommandToken, GetsUNCPathParameterWithSpacesAfterRelativeCommand)
{
	CString input = _T("tool.exe \\\\server\\share\\folder with spaces\\file.txt");
	CommandToken tok(input);

	int start = 0;
	int end = 0;
	ASSERT_TRUE(tok.GetPathParameterRange(start, end));
	EXPECT_EQ(_T("\\\\server\\share\\folder with spaces\\file.txt"), input.Mid(start, end - start));
}
