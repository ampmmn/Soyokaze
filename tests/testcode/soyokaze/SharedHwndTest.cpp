#include "stdafx.h"
#include "gtest/gtest.h"
#include "SharedHwnd.h"

// コンストラクタによって初期化されたHWNDを共有できること
TEST(SharedHwnd, canShareHandleInitializedByConstructor)
{
	HWND org = nullptr;
	{
		// アプリ起動中にユニットテストを実行すると、このテストによって共有メモリが上書きため、元の値をとっておく
		SharedHwnd org_hwnd;
		org = org_hwnd.GetHwnd();
	}

	HWND hwnd = (HWND)(size_t)0xdeadbeef;
	SharedHwnd h(hwnd);

	SharedHwnd h2;

	EXPECT_EQ(hwnd, h2.GetHwnd());

	if (org != nullptr) {
		// 上書きした値を復元する
		SharedHwnd restore(org);
	}
}
