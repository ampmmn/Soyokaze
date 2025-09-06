#include "stdafx.h"
#include "gtest/gtest.h"
#include "app/StartupParam.h"

TEST(StartupParam, canInit)
{
	TCHAR* argv[] = { (TCHAR*)_T("") };
	StartupParam param(1, argv);
}

TEST(StartupParam, HasRunComandReturnsFalse)
{
	TCHAR* argv[] = { (TCHAR*)_T("") };
	StartupParam param(1, argv);

	String value;
	EXPECT_FALSE(param.HasRunCommand(value));
	EXPECT_FALSE(param.HasPathToRegister(value));
	EXPECT_FALSE(param.HasHideOption());
}

TEST(StartupParam, HasRunComandReturnsTrue)
{
	// /c xxxx でコマンド指定オプションとして扱われる
	const TCHAR* argv[] = { _T(""), _T("-c"),  _T("aiueo") };
	StartupParam param(3, (TCHAR**)argv);

	String value;
	EXPECT_TRUE(param.HasRunCommand(value));
	EXPECT_EQ("aiueo", value);
}

TEST(StartupParam, HasRunComandReturnsTrue2)
{
	// /Runcommand=xxxx もコマンド指定オプションとして扱う
	const TCHAR* argv[] = { _T(""), _T("/Runcommand=aiueo") };
	StartupParam param(2, (TCHAR**)argv);

	String value;
	EXPECT_TRUE(param.HasRunCommand(value));
	EXPECT_EQ("aiueo", value);
}

TEST(StartupParam, HasPathToRegisterReturnsTrue)
{
	// 引数に実在するファイルを指定すると、登録ファイル

	TCHAR path[65536];
	GetModuleFileName(NULL, path, 65536);

	const TCHAR* argv[] = { _T(""), path };
	StartupParam param(2, (TCHAR**)argv);

	String value;
	EXPECT_TRUE(param.HasPathToRegister(value));
}

TEST(StartupParam, HasHideOption)
{
	const TCHAR* argv[] = { _T(""), _T("/Hide") };
	StartupParam param(2, (TCHAR**)argv);

	EXPECT_TRUE(param.HasHideOption());
}

