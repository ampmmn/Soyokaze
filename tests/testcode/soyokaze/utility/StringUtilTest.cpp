#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/StringUtil.h"

using namespace launcherapp::utility;

// StringUtil.cpp内のnamespaceに合わせてテストを記述します
// GetFirstLine関数のユニットテスト
TEST(StringUtilTest, GetFirstLine) {
    // Case 1: 標準的なCRLFを持つ文字列
    EXPECT_EQ(_T("first line"), GetFirstLine(_T("first line\r\nsecond line")));

    // Case 2: 標準的なLFを持つ文字列
    EXPECT_EQ(_T("first line"), GetFirstLine(_T("first line\nsecond line")));

    // Case 3: 改行コードがない場合、文字列全体を返す
    EXPECT_EQ(_T("single line"), GetFirstLine(_T("single line")));

    // Case 4: 空の文字列
    EXPECT_EQ(_T(""), GetFirstLine(_T("")));

    // Case 5: CRLFから始まる文字列
    EXPECT_EQ(_T(""), GetFirstLine(_T("\r\nsecond line")));

    // Case 6: LFから始まる文字列
    EXPECT_EQ(_T(""), GetFirstLine(_T("\nsecond line")));
    
    // Case 7: CRLFのみの文字列
    EXPECT_EQ(_T(""), GetFirstLine(_T("\r\n")));

    // Case 8: LFのみの文字列
    EXPECT_EQ(_T(""), GetFirstLine(_T("\n")));

    // Case 9: 複数の改行コードが混在する場合
    // 現在の実装は、文字列の先頭に近い改行コードよりも \r\n を優先して検索します。
    // このテストは、その現在の動作を検証するものです。
    EXPECT_EQ(_T("line1\nline2"), GetFirstLine(_T("line1\nline2\r\nline3")));
}

