#include "stdafx.h"
#include "gtest/gtest.h"
#include "AppProfile.h"
#include <regex>

TEST(AppProfile, GetDirPath)
{
	TCHAR path[1024];
	CAppProfile::GetDirPath(path, 1024);

	std::wregex re(LR"(^.:\\Users\\.+?\\\.bwlite)");

	EXPECT_TRUE(std::regex_match(path, re));
}

TEST(AppProfile, GetFilePath)
{
	TCHAR path[1024];
	CAppProfile::GetFilePath(path, 1024);

	std::wregex re(LR"(^.:\\Users\\.+?\\\.bwlite\\BWLite.ini)");

	EXPECT_TRUE(std::regex_match(path, re));
}
