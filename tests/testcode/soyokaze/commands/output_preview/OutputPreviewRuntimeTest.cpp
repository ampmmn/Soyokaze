#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/output_preview/OutputPreviewRuntime.h"

using launcherapp::commands::output_preview::OutputPreviewRuntime;

TEST(OutputPreviewRuntimeTest, AllowsOnlyOneActiveExecutionOrPreview)
{
	auto runtime = OutputPreviewRuntime::Get();
	ASSERT_TRUE(runtime->TryBegin(_T("preview-session"), _T("preview"), _T("preview --arg")));
	EXPECT_TRUE(runtime->IsBusy());
	EXPECT_FALSE(runtime->TryBegin(_T("another-session"), _T("another"), _T("another")));

	ASSERT_TRUE(runtime->Publish(_T("preview-session"), _T("output"), _T(""), 0, _T("C:\\Tools\\tool.exe")));
	OutputPreviewRuntime::Result result;
	ASSERT_TRUE(runtime->Fetch(_T("preview-session"), result));
	EXPECT_EQ(result.mName, _T("preview-session"));
	EXPECT_EQ(result.mCommandName, _T("preview"));
	EXPECT_EQ(result.mInputText, _T("preview --arg"));
	EXPECT_EQ(result.mExecutablePath, _T("C:\\Tools\\tool.exe"));
	EXPECT_EQ(result.mText, _T("output"));
	EXPECT_TRUE(runtime->IsResultForCommand(_T("PREVIEW"), _T("preview --arg")));
	EXPECT_FALSE(runtime->IsResultForCommand(_T("PREVIEW"), _T("preview")));
	EXPECT_FALSE(runtime->IsResultForCommand(_T("another"), _T("another")));
	EXPECT_FALSE(runtime->TryBegin(_T("another-session"), _T("another"), _T("another")));

	runtime->Close(_T("preview-session"));
	EXPECT_FALSE(runtime->IsBusy());
	EXPECT_FALSE(runtime->IsResultForCommand(_T("preview"), _T("preview --arg")));
	EXPECT_TRUE(runtime->TryBegin(_T("another-session"), _T("another"), _T("another")));
	runtime->Close(_T("another-session"));
}

TEST(OutputPreviewRuntimeTest, RemovesTemporaryFilesWhenTheResultCloses)
{
	TCHAR tempDirectory[MAX_PATH_NTFS]{};
	DWORD directoryLength = GetTempPath(_countof(tempDirectory), tempDirectory);
	ASSERT_NE(0u, directoryLength);
	TCHAR tempFile[MAX_PATH_NTFS]{};
	ASSERT_NE(0u, GetTempFileName(tempDirectory, _T("spr"), 0, tempFile));

	auto runtime = OutputPreviewRuntime::Get();
	ASSERT_TRUE(runtime->TryBegin(_T("file-session"), _T("file"), _T("file")));
	ASSERT_TRUE(runtime->AddTemporaryFile(_T("file-session"), tempFile));
	ASSERT_TRUE(runtime->Publish(_T("file-session"), _T(""), tempFile, 1, _T("")));
	runtime->Close(_T("file-session"));

	EXPECT_EQ(INVALID_FILE_ATTRIBUTES, GetFileAttributes(tempFile));
}
