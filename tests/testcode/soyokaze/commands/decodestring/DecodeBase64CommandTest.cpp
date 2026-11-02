#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/decodestring/DecodeBase64Command.h"
#include "commands/core/PreviewProvider.h"
#include "core/IFIDDefine.h"
#include "matcher/WholeMatchPattern.h"
#include "utility/RefPtr.h"

using DecodeBase64Command = launcherapp::commands::decodestring::DecodeBase64Command;

TEST(DecodeBase64CommandTest, MatchProvidesDecodedPreview)
{
	DecodeBase64Command command;
	auto pattern = WholeMatchPattern::Create(_T("VGhpcyBpcyBhIGRlY29kaW5nIHRlc3Q="));
	ASSERT_NE(nullptr, pattern);
	EXPECT_EQ(Pattern::PartialMatch, command.Match(pattern));
	pattern->Release();

	EXPECT_EQ(_T("This is a decoding test"), command.GetName());

	RefPtr<launcherapp::core::PreviewProvider> provider;
	ASSERT_TRUE(command.QueryInterface(IFID_PREVIEWPROVIDER, (void**)&provider));
	ASSERT_NE(nullptr, provider.get());
	EXPECT_NE(nullptr, provider->FetchPreviewWindow());
}
