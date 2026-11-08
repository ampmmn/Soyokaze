#include "stdafx.h"
#include "utility/FileAssociationResolver.h"
#include "utility/CommandLineArgument.h"

using launcherapp::utility::FileAssociationResolver;

TEST(FileAssociationResolverTest, ParsesQuotedExecutableAndArguments)
{
	CString executablePath;
	CString arguments;
	ASSERT_TRUE(FileAssociationResolver::ParseAssociationCommand(
		_T("\"C:\\Program Files\\Viewer\\viewer.exe\" \"%1\" %*"), executablePath, arguments));

	EXPECT_EQ(_T("C:\\Program Files\\Viewer\\viewer.exe"), executablePath);
	EXPECT_EQ(_T("\"%1\" %*"), arguments);
}

TEST(FileAssociationResolverTest, ParsesUnquotedExecutable)
{
	CString executablePath;
	CString arguments;
	ASSERT_TRUE(FileAssociationResolver::ParseAssociationCommand(
		_T("C:\\Tools\\viewer.exe /open %1"), executablePath, arguments));

	EXPECT_EQ(_T("C:\\Tools\\viewer.exe"), executablePath);
	EXPECT_EQ(_T("/open %1"), arguments);
}

TEST(FileAssociationResolverTest, RejectsMalformedAssociationCommand)
{
	CString executablePath;
	CString arguments;
	EXPECT_FALSE(FileAssociationResolver::ParseAssociationCommand(_T("\"unterminated"), executablePath, arguments));
	EXPECT_FALSE(FileAssociationResolver::ParseAssociationCommand(_T(" "), executablePath, arguments));
}

TEST(FileAssociationResolverTest, LaunchesBatchAndCommandFilesThroughCommandInterpreter)
{
	TCHAR temporaryDirectory[MAX_PATH_NTFS]{};
	DWORD directoryLength = GetTempPath(_countof(temporaryDirectory), temporaryDirectory);
	ASSERT_NE(0u, directoryLength);

	FileAssociationResolver resolver;
	for (const auto& extension : { _T("bat"), _T("cmd") }) {
		CString batchPath;
		batchPath.Format(_T("%sSoyokaze Batch %lu.%s"), temporaryDirectory, GetCurrentProcessId(), extension);
		HANDLE file = CreateFile(batchPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		ASSERT_NE(INVALID_HANDLE_VALUE, file);
		const char script[] = "@echo off\r\nexit /b 0\r\n";
		DWORD written = 0;
		BOOL writeSucceeded = WriteFile(file, script, sizeof(script) - 1, &written, nullptr);
		CloseHandle(file);
		ASSERT_NE(FALSE, writeSucceeded);
		ASSERT_EQ(sizeof(script) - 1, written);

		CString executablePath;
		CString arguments;
		CString inputArguments = launcherapp::utility::QuoteArgument(_T("argument with spaces"));
		ASSERT_TRUE(resolver.Resolve(batchPath, inputArguments, executablePath, arguments));

		int separator = executablePath.ReverseFind(_T('\\'));
		EXPECT_EQ(0, executablePath.Mid(separator + 1).CompareNoCase(_T("cmd.exe")));
		CString expectedArguments = _T("/d /s /c \"") + launcherapp::utility::QuoteArgument(batchPath) +
			_T(" ") + inputArguments + _T("\"");
		EXPECT_EQ(arguments, expectedArguments);

		CString commandLine = launcherapp::utility::QuoteArgument(executablePath) + _T(" ") + arguments;
		std::vector<TCHAR> commandLineBuffer(commandLine.GetLength() + 1);
		_tcscpy_s(commandLineBuffer.data(), commandLineBuffer.size(), commandLine);
		STARTUPINFO startupInfo{};
		startupInfo.cb = sizeof(startupInfo);
		PROCESS_INFORMATION processInfo{};
		BOOL started = CreateProcess(executablePath, commandLineBuffer.data(), nullptr, nullptr, FALSE,
			CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo);
		ASSERT_NE(FALSE, started);
		DWORD waitResult = WaitForSingleObject(processInfo.hProcess, 5000);
		if (waitResult != WAIT_OBJECT_0) {
			TerminateProcess(processInfo.hProcess, 1);
			WaitForSingleObject(processInfo.hProcess, INFINITE);
		}
		DWORD exitCode = 1;
		BOOL gotExitCode = GetExitCodeProcess(processInfo.hProcess, &exitCode);
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
		DeleteFile(batchPath);
		ASSERT_EQ(WAIT_OBJECT_0, waitResult);
		ASSERT_NE(FALSE, gotExitCode);
		EXPECT_EQ(0u, exitCode);
	}
}
