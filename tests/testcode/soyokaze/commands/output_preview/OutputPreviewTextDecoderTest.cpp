#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/output_preview/OutputPreviewTextDecoder.h"

using launcherapp::commands::output_preview::OutputPreviewTextDecoder;

TEST(OutputPreviewTextDecoderTest, DecodesValidUtf8Text)
{
	const char text[] = "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E";
	CString decoded;

	OutputPreviewTextDecoder::Decode(text, decoded);

	EXPECT_EQ(decoded, _T("日本語"));
}

TEST(OutputPreviewTextDecoderTest, DecodesNonUtf8TextAsShiftJis)
{
	const char text[] = "\x93\xFA\x96\x7B\x8C\xEA";
	CString decoded;

	OutputPreviewTextDecoder::Decode(text, decoded);

	EXPECT_EQ(decoded, _T("日本語"));
}

TEST(OutputPreviewTextDecoderTest, DecodesNullAsEmptyText)
{
	CString decoded = _T("existing");

	OutputPreviewTextDecoder::Decode(nullptr, decoded);

	EXPECT_TRUE(decoded.IsEmpty());
}
