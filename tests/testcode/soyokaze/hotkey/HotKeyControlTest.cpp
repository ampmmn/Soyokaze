#include "stdafx.h"
#include "gtest/gtest.h"
#include "hotkey/HotKeyControl.h"

class HotKeyControlForTest : public HotKeyControl
{
public:
	HotKeyControlForTest() : HotKeyControl() {}

	UINT GetNotifyIdForTest() { return GetNotifyId(); }

	// 呼び出し検証用にオーバーライド
	void SetText(const CString& text) override {
		mSetTextCalled = true;
		mLastText = text;
	}
	void SetCurBanner(const CString& bannerMsg) override {
		mSetCurBannerCalledCount++;
		mLastBannerMsg = bannerMsg;
	}
	void Notify(UINT id) override {
		mNotifyCalled = true;
		mLastNotifyId = id;
	}

	bool mSetTextCalled = false;
	CString mLastText;
	int mSetCurBannerCalledCount = 0;
	CString mLastBannerMsg;
	bool mNotifyCalled = false;
	UINT mLastNotifyId = 0;

	void Reset() {
		mSetTextCalled = false;
		mLastText = _T("");
		mSetCurBannerCalledCount = 0;
		mLastBannerMsg = _T("");
		mNotifyCalled = false;
		mLastNotifyId = 0;
	}
};

// SetNotifyIdで設定したIDがGetNotifyIdで取得できること
TEST(HotKeyControl, testSetNotifyId)
{
	HotKeyControlForTest ctrl;
	ctrl.SetNotifyId(1234);
	EXPECT_EQ(1234, ctrl.GetNotifyIdForTest());
}

// UpdateContentをよびだしたとき、初回はSetCurBannerが呼ばれること
// 2回目の呼び出しではSetCurBannerはよばれないこと
// UpdateContentを呼び出したとき、SetTextが呼ばれること。テキストはUpdateContentで呼び出したものと同じであること
TEST(HotKeyControl, testUpdateContent)
{
	HotKeyControlForTest ctrl;

	// 初回呼び出し
	ctrl.UpdateContent(_T("TEST_TEXT"));
	EXPECT_EQ(1, ctrl.mSetCurBannerCalledCount);
	EXPECT_TRUE(ctrl.mSetTextCalled);
	EXPECT_STREQ(_T("TEST_TEXT"), ctrl.mLastText);

	ctrl.Reset();

	// 2回目呼び出し
	ctrl.UpdateContent(_T("NEXT_TEXT"));
	EXPECT_EQ(0, ctrl.mSetCurBannerCalledCount); // 呼ばれないはず
	EXPECT_TRUE(ctrl.mSetTextCalled);
	EXPECT_STREQ(_T("NEXT_TEXT"), ctrl.mLastText);
}

// OnLButtonDownが呼ばれたとき..
// mNotifyIdが0のときはNotifyが行われないこと
// mNotifyIdが0でないのときはNotifyが行われること
TEST(HotKeyControl, testOnLButtonDown)
{
	HotKeyControlForTest ctrl;

	// mNotifyIdが0
	ctrl.SetNotifyId(0);
	ctrl.OnLButtonDown(0, CPoint(0,0));
	EXPECT_FALSE(ctrl.mNotifyCalled);

	ctrl.Reset();

	// mNotifyIdが0でない
	ctrl.SetNotifyId(123);
	ctrl.OnLButtonDown(0, CPoint(0,0));
	EXPECT_TRUE(ctrl.mNotifyCalled);
	EXPECT_EQ(123, ctrl.mLastNotifyId);
}

// OnSetFocusがよばれたとき..
// mNotifyIdが0のときはNotifyが行われないこと
// mNotifyIdが0でなく、oldWindowがnullptrのときは通知が行われないこと
// mNotifyIdが0でなく、oldWindowがnullptrでないときは通知が行われること
TEST(HotKeyControl, testOnSetFocus)
{
	HotKeyControlForTest ctrl;

	// mNotifyIdが0
	ctrl.SetNotifyId(0);
	ctrl.OnSetFocus(nullptr);
	EXPECT_FALSE(ctrl.mNotifyCalled);

	ctrl.Reset();

	// mNotifyIdが0でなく、oldWindowがnullptr
	ctrl.SetNotifyId(123);
	ctrl.OnSetFocus(nullptr);
	EXPECT_FALSE(ctrl.mNotifyCalled);

	ctrl.Reset();

	// mNotifyIdが0でなく、oldWindowがnullptrでない
	// 適当なHWNDを持つCWndを作成
	CWnd dummyWnd;
	ctrl.SetNotifyId(456);
	ctrl.OnSetFocus(&dummyWnd);
	EXPECT_TRUE(ctrl.mNotifyCalled);
	EXPECT_EQ(456, ctrl.mLastNotifyId);
}

// OnKeyDown, OnSysKeyDown, OnCharを呼ぶテスト(呼ぶだけでOK)
TEST(HotKeyControl, testMessageHandlers)
{
	HotKeyControlForTest ctrl;
	ctrl.OnKeyDown(0, 0, 0);
	ctrl.OnSysKeyDown(0, 0, 0);
	ctrl.OnChar(0, 0, 0);
	// 落ちなければOK
}
