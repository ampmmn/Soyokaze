#include "stdafx.h"
#include "utility/ProcessPath.h"
#include "utility/ScopeExit.h"

TEST(ProcessPathTest, GetsFullCommandLineForCurrentProcess)
{
	CString commandLine = ProcessPath::GetFullCommandLine(GetCurrentProcessId());
	ASSERT_FALSE(commandLine.IsEmpty());

	std::vector<TCHAR> pathBuffer(32768);
	DWORD pathLength = GetModuleFileName(nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
	ASSERT_NE(0, pathLength);
	CString modulePath(pathBuffer.data(), pathLength);

	CString quotedModulePath;
	quotedModulePath.Format(_T("\"%s\""), (LPCTSTR)modulePath);
	EXPECT_TRUE(commandLine.Left(modulePath.GetLength()).CompareNoCase(modulePath) == 0 ||
		commandLine.Left(quotedModulePath.GetLength()).CompareNoCase(quotedModulePath) == 0);
}

TEST(ProcessPathTest, RejectsInvalidProcessId)
{
	EXPECT_THROW(ProcessPath::GetFullCommandLine(0), ProcessPath::Exception);
}

TEST(ProcessPathTest, GetsCommandLineFromProcessWithDifferentBitness)
{
	CString targetPath;
#ifdef _WIN64
	std::vector<TCHAR> directoryBuffer(32768);
	UINT directoryLength = GetSystemWow64Directory(directoryBuffer.data(), static_cast<UINT>(directoryBuffer.size()));
	if (directoryLength == 0 || directoryLength >= directoryBuffer.size()) {
		GTEST_SKIP();
	}
	targetPath = directoryBuffer.data();
	targetPath += _T("\\cmd.exe");
#else
	BOOL currentIsWow64 = FALSE;
	if (IsWow64Process(GetCurrentProcess(), &currentIsWow64) == FALSE || currentIsWow64 == FALSE) {
		GTEST_SKIP();
	}
	std::vector<TCHAR> directoryBuffer(32768);
	UINT directoryLength = GetWindowsDirectory(directoryBuffer.data(), static_cast<UINT>(directoryBuffer.size()));
	if (directoryLength == 0 || directoryLength >= directoryBuffer.size()) {
		GTEST_SKIP();
	}
	targetPath = directoryBuffer.data();
	targetPath += _T("\\Sysnative\\cmd.exe");
#endif
	if (GetFileAttributes(targetPath) == INVALID_FILE_ATTRIBUTES) {
		GTEST_SKIP();
	}

	CString commandLine;
	commandLine.Format(_T("\"%s\" /c echo SoyokazeCrossBitnessMarker"), (LPCTSTR)targetPath);
	std::vector<TCHAR> commandLineBuffer(commandLine.GetLength() + 1);
	_tcscpy_s(commandLineBuffer.data(), commandLineBuffer.size(), commandLine);

	STARTUPINFO startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInfo{};
	if (CreateProcess(
		targetPath,
		commandLineBuffer.data(),
		nullptr,
		nullptr,
		FALSE,
		CREATE_SUSPENDED | CREATE_NO_WINDOW,
		nullptr,
		nullptr,
		&startupInfo,
		&processInfo) == FALSE) {
		GTEST_SKIP();
	}
	utility::ScopeExit processGuard([&processInfo]() {
		TerminateProcess(processInfo.hProcess, 0);
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	});

	CString actualCommandLine = ProcessPath::GetFullCommandLine(processInfo.dwProcessId);
	EXPECT_NE(-1, actualCommandLine.Find(targetPath));
	EXPECT_NE(-1, actualCommandLine.Find(_T("SoyokazeCrossBitnessMarker")));
}
