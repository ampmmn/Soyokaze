#include "stdafx.h"
#include "gtest/gtest.h"
#include "SharedHwnd.h"

// コンストラクタによって初期化されたHWNDを共有できること
TEST(SharedHwnd, canShareHandleInitializedByConstructor)
{
	HWND hwnd = (HWND)(size_t)0xdeadbeef;
	SharedHwnd h(hwnd);

	SharedHwnd h2;

	EXPECT_EQ(hwnd, h2.GetHwnd());
}
