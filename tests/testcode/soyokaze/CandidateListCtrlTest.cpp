#include "stdafx.h"
#include "gtest/gtest.h"
#include "features/main/CandidateListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TEST(CandidateListCtrl, RecalculatesColumnsWhenControlIsResized)
{
	ASSERT_TRUE(AfxWinInit(::GetModuleHandle(nullptr), nullptr, ::GetCommandLine(), 0));

	CWnd parent;
	CString className = AfxRegisterWndClass(0);
	ASSERT_TRUE(parent.CreateEx(
		0,
		className,
		_T(""),
		WS_POPUP,
		CRect(0, 0, 640, 480),
		nullptr,
		0
	));

	CandidateListCtrl list;
	ASSERT_TRUE(list.Create(WS_CHILD | LVS_REPORT, CRect(0, 0, 500, 200), &parent, 1));
	list.InitColumns();

	list.SetWindowPos(nullptr, 0, 0, 320, 200, SWP_NOZORDER | SWP_NOACTIVATE);

	CRect clientRect;
	list.GetClientRect(&clientRect);
	int expectedWidth = clientRect.Width() - GetSystemMetrics(SM_CXVSCROLL);

	ASSERT_NE(nullptr, list.GetHeaderCtrl());
	int columnsWidth = 0;
	int columnsCount = list.GetHeaderCtrl()->GetItemCount();
	for (int index = 0; index < columnsCount; ++index) {
		columnsWidth += list.GetColumnWidth(index);
	}

	EXPECT_EQ(expectedWidth, columnsWidth);
}
