#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/ShellExecCommand.h"


TEST(ShellExecCommand, ExpandEnv1)
{
	CString str(_T("https://www.google.com/search?q=$1"));
	ShellExecCommand::ExpandEnv(str);

	EXPECT_EQ(_T("https://www.google.com/search?q=$1"), str);
}
