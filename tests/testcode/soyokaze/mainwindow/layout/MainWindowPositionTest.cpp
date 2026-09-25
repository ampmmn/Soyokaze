#include "stdafx.h"
#include "gtest/gtest.h"
#include "mainwindow/layout/MainWindowPosition.h"
#include "utility/Base64.h"

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

TEST(MainWindowPositionTest, SyncPosition_DoesNotShowHiddenWindow)
{
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T(""), WS_OVERLAPPEDWINDOW,
	                          0, 0, 400, 300, nullptr, nullptr, AfxGetInstanceHandle(), nullptr);
	ASSERT_NE(hwnd, nullptr);
	ASSERT_FALSE(IsWindowVisible(hwnd));

	MainWindowPosition pos;
	EXPECT_TRUE(pos.SyncPosition(hwnd));
	EXPECT_FALSE(IsWindowVisible(hwnd));
	EXPECT_EQ(SW_SHOWNORMAL, pos.GetPosition().showCmd);

	DestroyWindow(hwnd);
}

TEST(MainWindowPositionTest, MonitorConfigurationIdentifier_IsIndependentOfEnumerationOrder)
{
	RECT leftMonitor{ -1920, 0, 0, 1080 };
	RECT rightMonitor{ 0, 0, 1920, 1080 };
	std::vector<RECT> monitors{ leftMonitor, rightMonitor };
	std::vector<RECT> reversed{ rightMonitor, leftMonitor };

	CString identifier = WindowPosition::CreateMonitorConfigurationIdentifier(monitors);
	EXPECT_EQ(40, identifier.GetLength());
	EXPECT_STREQ((LPCTSTR)identifier, (LPCTSTR)WindowPosition::CreateMonitorConfigurationIdentifier(reversed));
}

TEST(MainWindowPositionTest, WindowPlacementData_ValidatesSizeAndLength)
{
	WINDOWPLACEMENT expected{};
	expected.length = sizeof(WINDOWPLACEMENT);
	expected.showCmd = SW_SHOWNORMAL;
	expected.rcNormalPosition = RECT{ 10, 20, 300, 400 };
	std::vector<uint8_t> bytes(sizeof(WINDOWPLACEMENT));
	memcpy(bytes.data(), &expected, sizeof(WINDOWPLACEMENT));

	WINDOWPLACEMENT actual{};
	EXPECT_TRUE(WindowPosition::IsValidWindowPlacementData(bytes, actual));
	EXPECT_EQ(expected.rcNormalPosition.left, actual.rcNormalPosition.left);
	EXPECT_EQ(expected.rcNormalPosition.bottom, actual.rcNormalPosition.bottom);

	actual.length = 0;
	memcpy(bytes.data(), &actual, sizeof(WINDOWPLACEMENT));
	EXPECT_FALSE(WindowPosition::IsValidWindowPlacementData(bytes, actual));
	bytes.pop_back();
	EXPECT_FALSE(WindowPosition::IsValidWindowPlacementData(bytes, actual));
}

TEST(MainWindowPositionTest, EmptyBase64Data_IsRejected)
{
	std::vector<uint8_t> bytes;
	EXPECT_FALSE(utility::base64::DecodeBase64(_T(" "), bytes));
}

