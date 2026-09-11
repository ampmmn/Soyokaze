#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/shelluri/ShellUriCommand.h"
#include "matcher/WholeMatchPattern.h"

using ShellUriCommand = launcherapp::commands::shelluri::ShellUriCommand;

namespace {

int Match(ShellUriCommand& command, LPCTSTR text)
{
	auto pattern = WholeMatchPattern::Create(text);
	int result = command.Match(pattern);
	pattern->Release();
	return result;
}

}

TEST(ShellUriCommandTest, MatchValidUri)
{
	ShellUriCommand command;

	EXPECT_EQ(Pattern::WholeMatch, Match(command, _T("shell:startup")));
	EXPECT_EQ(_T("shell:startup"), command.GetName());
	EXPECT_EQ(_T("shell:startup"), command.GetDescription());
	EXPECT_EQ(Pattern::WholeMatch, Match(command, _T("SHELL:Downloads")));
	EXPECT_EQ(_T("SHELL:Downloads"), command.GetName());
}

TEST(ShellUriCommandTest, RejectsInvalidScheme)
{
	ShellUriCommand command;

	EXPECT_EQ(Pattern::Mismatch, Match(command, _T("shell :startup")));
	EXPECT_EQ(Pattern::Mismatch, Match(command, _T("shell_test:startup")));
	EXPECT_EQ(Pattern::Mismatch, Match(command, _T("1shell:startup")));
	EXPECT_EQ(Pattern::Mismatch, Match(command, _T("+shell:startup")));
	EXPECT_EQ(Pattern::Mismatch, Match(command, _T("http://example.com")));
	EXPECT_EQ(Pattern::Mismatch, Match(command, _T("shell:")));
}

TEST(ShellUriCommandTest, RejectsWhitespace)
{
	ShellUriCommand command;

	EXPECT_EQ(Pattern::Mismatch, Match(command, _T("shell:common startup")));
	EXPECT_EQ(Pattern::Mismatch, Match(command, _T("shell:startup\n")));
}
