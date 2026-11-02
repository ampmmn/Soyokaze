#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/decodestring/DecodeUriCommand.h"
#include "commands/core/PreviewProvider.h"
#include "core/IFIDDefine.h"
#include "matcher/WholeMatchPattern.h"
#include "utility/RefPtr.h"

using DecodeUriCommand = launcherapp::commands::decodestring::DecodeUriCommand;

TEST(DecodeUriCommandTest, MatchProvidesDecodedPreview)
{
	DecodeUriCommand command;
	auto pattern = WholeMatchPattern::Create(_T("Hello%20World%21"));
	ASSERT_NE(nullptr, pattern);
	EXPECT_EQ(Pattern::PartialMatch, command.Match(pattern));
	pattern->Release();

	EXPECT_EQ(_T("Hello World!"), command.GetName());

	RefPtr<launcherapp::core::PreviewProvider> provider;
	ASSERT_TRUE(command.QueryInterface(IFID_PREVIEWPROVIDER, (void**)&provider));
	ASSERT_NE(nullptr, provider.get());
	EXPECT_NE(nullptr, provider->FetchPreviewWindow());
}
