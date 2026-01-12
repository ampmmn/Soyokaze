#include "stdafx.h"
#include "gtest/gtest.h"
#include "mainwindow/layout/MainWindowPosition.h"

TEST(MainWindowPositionTest, Constructor_Default)
{
    MainWindowPosition pos;
    // 何も例外が出ないことを確認
    SUCCEED();
}

TEST(MainWindowPositionTest, Constructor_WithName)
{
    MainWindowPosition pos(_T("TestWindow"));
    SUCCEED();
}

TEST(MainWindowPositionTest, SyncPosition_ReturnsBool)
{
    MainWindowPosition pos;
    HWND hwnd = nullptr;
    EXPECT_FALSE(pos.SyncPosition(hwnd));
}

