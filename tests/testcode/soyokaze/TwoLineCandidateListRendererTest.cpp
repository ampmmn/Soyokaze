#include "stdafx.h"
#include "gtest/gtest.h"
#include "features/main/StandardCandidateListRenderer.h"
#include "features/main/TwoLineCandidateListRenderer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TEST(TwoLineCandidateListRenderer, UsesCompactLineSpacing)
{
	StandardCandidateListRenderer standardRenderer;
	TwoLineCandidateListRenderer twoLineRenderer;
	standardRenderer.SetTextMetrics(16, 22, 20);
	twoLineRenderer.SetTextMetrics(16, 22, 20);

	EXPECT_EQ(20, standardRenderer.GetItemHeight());
	EXPECT_EQ(34, twoLineRenderer.GetItemHeight());
}
