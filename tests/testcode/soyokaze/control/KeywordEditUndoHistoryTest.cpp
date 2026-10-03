#include "stdafx.h"
#include "control/KeywordEditUndoHistory.h"

TEST(KeywordEditUndoHistoryTest, UndoRestoresPreviousTextAndStopsAtInitialText)
{
	KeywordEditUndoHistory history;
	history.Reset(_T(""));

	EXPECT_TRUE(history.RecordChange(_T("a")));
	EXPECT_TRUE(history.RecordChange(_T("ab")));

	CString text;
	ASSERT_TRUE(history.Undo(text));
	EXPECT_TRUE(text == _T("a"));
	ASSERT_TRUE(history.Undo(text));
	EXPECT_TRUE(text.IsEmpty());
	EXPECT_FALSE(history.Undo(text));
}

TEST(KeywordEditUndoHistoryTest, KeepsOnlyTheMostRecentSixteenChanges)
{
	KeywordEditUndoHistory history;
	history.Reset(_T("start"));
	for (int i = 0; i < 20; ++i) {
		CString text;
		text.Format(_T("value%d"), i);
		history.RecordChange(text);
	}

	CString text;
	for (int i = 19; i >= 4; --i) {
		ASSERT_TRUE(history.Undo(text));
		CString expected;
		expected.Format(_T("value%d"), i - 1);
		EXPECT_TRUE(expected == text);
	}
	EXPECT_FALSE(history.Undo(text));
}

TEST(KeywordEditUndoHistoryTest, ResetClearsHistoryAndSetsNewBaseline)
{
	KeywordEditUndoHistory history;
	history.Reset(_T("before"));
	history.RecordChange(_T("changed"));
	history.Reset(_T("cleared"));

	CString text;
	EXPECT_FALSE(history.Undo(text));
	EXPECT_TRUE(history.RecordChange(_T("after")));
	ASSERT_TRUE(history.Undo(text));
	EXPECT_TRUE(text == _T("cleared"));
}
