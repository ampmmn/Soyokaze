#include "stdafx.h"
#include "gtest/gtest.h"
#include "control/SystemColorScheme.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TEST(SystemColorScheme, UsesSystemColorsForSecondLineText)
{
	SystemColorScheme colorScheme;

	EXPECT_EQ(GetSysColor(COLOR_GRAYTEXT), colorScheme.GetListText2Color());
	EXPECT_EQ(GetSysColor(COLOR_HIGHLIGHTTEXT), colorScheme.GetListHighlightText2Color());
}
