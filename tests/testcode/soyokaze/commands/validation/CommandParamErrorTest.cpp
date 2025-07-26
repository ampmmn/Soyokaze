#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/validation/CommandParamError.h"
#include "commands/validation/CommandParamErrorCode.h"

using namespace launcherapp::commands::validation;

TEST(CommandParamErrorTest, DefaultConstructor)
{
    CommandParamError err;
    CString str = err.ToString();
    EXPECT_TRUE(str.IsEmpty());
}

TEST(CommandParamErrorTest, ErrorCodeConstructor)
{
    CommandParamError err(ActivateWindow_CaptionAndClassBothEmpty);
    CString str = err.ToString();
    EXPECT_EQ(_T("ウインドウタイトルかウインドウクラスを入力してください"), str);
}

