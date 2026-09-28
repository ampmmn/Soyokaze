#include "stdafx.h"
#include "gtest/gtest.h"
#include "core/LauncherProcessContext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TEST(LauncherProcessContext, TracksRestartRequirement)
{
	auto context = launcherapp::core::LauncherProcessContext::GetInstance();
	EXPECT_FALSE(context->IsRestartRequired());

	context->MarkRestartRequired();
	EXPECT_TRUE(context->IsRestartRequired());
}
