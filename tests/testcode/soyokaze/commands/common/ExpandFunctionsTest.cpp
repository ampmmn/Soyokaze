#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/common/ExpandFunctions.h"

using namespace launcherapp::commands::common;

// ExpandEnv関数削除によりテスト無効化
//TEST(ExpandFunctions, ExpandEnv1)
//{
//	CString str(_T("https://www.google.com/search?q=$1"));
//	ExpandEnv(str);
//
//	EXPECT_EQ(_T("https://www.google.com/search?q=$1"), str);
//}
