#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/shellexecute/ShellExecCommandParam.h"

using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;
using CommandParam = launcherapp::commands::shellexecute::CommandParam;

TEST(ShellExecCommandTest, ResolveNormalPath)
{
	ShellExecCommand command;
	command.SetPath(_T("C:\\Program Files\\app.exe"));

	CString value(_T("before"));

	EXPECT_TRUE(command.CanResolve());
	EXPECT_TRUE(command.Resolve(value));
	EXPECT_EQ(_T("C:\\Program Files\\app.exe"), value);
}

TEST(ShellExecCommandTest, CannotResolveWhenNoParameterPathExists)
{
	ShellExecCommand command;
	CommandParam param;
	param.mNormalAttr.mPath = _T("C:\\app.exe");
	param.mNoParamAttr.mPath = _T("C:\\app-no-param.exe");
	command.SetParam(param);

	CString value(_T("before"));

	EXPECT_FALSE(command.CanResolve());
	EXPECT_FALSE(command.Resolve(value));
	EXPECT_EQ(_T("before"), value);
}

TEST(ShellExecCommandTest, AcceptsArgumentsWhenPlaceholderExists)
{
	ShellExecCommand command;
	command.SetArgument(_T("$1"));

	EXPECT_TRUE(command.IsAcceptArguments());
}

