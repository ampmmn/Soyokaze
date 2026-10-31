#include "stdafx.h"
#include "commands/builtin/KillTaskCommand.h"
#include "commands/builtin/KillTaskCandidateCommand.h"
#include "actions/core/Action.h"
#include "utility/RefPtr.h"
#include "utility/ScopeExit.h"
#include "matcher/PartialMatchPattern.h"
#include "commands/core/CommandQueryItemList.h"

using KillTaskCommand = launcherapp::commands::builtin::KillTaskCommand;
using KillTaskCandidateCommand = launcherapp::commands::builtin::KillTaskCandidateCommand;
using Action = launcherapp::actions::core::Action;

TEST(KillTaskCommandTest, ProvidesCommandInformation)
{
	KillTaskCommand command;

	EXPECT_EQ(_T("kill"), command.GetName());
	EXPECT_EQ(_T("【タスクを終了する】"), command.GetDescription());
}

TEST(KillTaskCommandTest, ProvidesProcessCandidateInformation)
{
	KillTaskCandidateCommand command(
		1234,
		_T("example.exe"),
		_T("C:\\Windows\\example.exe"),
		_T("C:\\Windows\\example.exe -test"),
		false,
		1,
		false);

	EXPECT_EQ(_T("kill example.exe"), command.GetName());
	EXPECT_EQ(_T("PID:1234 (C:\\Windows\\example.exe -test)"), command.GetDescription());
	EXPECT_EQ(_T("タスク終了コマンド"), command.GetTypeDisplayName());
}

TEST(KillTaskCommandTest, DescribesProcessesWithoutTerminationPermission)
{
	KillTaskCandidateCommand command(
		1234,
		_T("example.exe"),
		_T(""),
		_T(""),
		true,
		0,
		false);

	EXPECT_EQ(
		_T("PID:1234 (example.exe)（権限不足のため終了できません）"),
		command.GetDescription());
}

TEST(KillTaskCommandTest, DoesNothingForProcessesWithoutTerminationPermission)
{
	KillTaskCandidateCommand command(
		1234,
		_T("example.exe"),
		_T(""),
		_T(""),
		true,
		0,
		true);
	Action* action = nullptr;

	ASSERT_TRUE(command.GetAction(HOTKEY_ATTR(0, VK_RETURN), &action));
	ASSERT_NE(nullptr, action);
	EXPECT_EQ(_T("タスク終了"), action->GetDisplayName());
	EXPECT_TRUE(action->Perform(nullptr));
	action->Release();
}

TEST(KillTaskCommandTest, ClonesProcessCandidateInformation)
{
	KillTaskCandidateCommand command(
		1234,
		_T("example.exe"),
		_T(""),
		_T(""),
		true,
		0,
		false);
	RefPtr<launcherapp::core::Command> clone(command.Clone());

	EXPECT_EQ(command.GetName(), clone->GetName());
	EXPECT_EQ(command.GetDescription(), clone->GetDescription());
}

TEST(KillTaskCommandTest, FallsBackToImageNameWhenCommandLineIsUnavailable)
{
	KillTaskCandidateCommand command(
		1234,
		_T("example.exe"),
		_T(""),
		_T(""),
		false,
		1,
		false);

	EXPECT_EQ(_T("PID:1234 (example.exe)"), command.GetDescription());
}

TEST(KillTaskCommandTest, FindsProcessByPid)
{
	std::vector<TCHAR> systemDirectory(32768);
	UINT directoryLength = GetSystemDirectory(systemDirectory.data(), static_cast<UINT>(systemDirectory.size()));
	ASSERT_NE(0, directoryLength);
	ASSERT_LT(directoryLength, systemDirectory.size());
	CString executablePath(systemDirectory.data(), directoryLength);
	executablePath += _T("\\cmd.exe");

	CString childCommandLine;
	childCommandLine.Format(_T("\"%s\" /c echo SoyokazeKillTaskPidMatch"), (LPCTSTR)executablePath);
	std::vector<TCHAR> childCommandLineBuffer(childCommandLine.GetLength() + 1);
	_tcscpy_s(childCommandLineBuffer.data(), childCommandLineBuffer.size(), childCommandLine);

	STARTUPINFO startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInfo{};
	ASSERT_NE(FALSE, CreateProcess(
		executablePath,
		childCommandLineBuffer.data(),
		nullptr,
		nullptr,
		FALSE,
		CREATE_SUSPENDED | CREATE_NO_WINDOW,
		nullptr,
		nullptr,
		&startupInfo,
		&processInfo));
	utility::ScopeExit processGuard([&processInfo]() {
		TerminateProcess(processInfo.hProcess, 0);
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	});

	CString queryText;
	queryText.Format(_T("kill %lu"), processInfo.dwProcessId);
	RefPtr<PartialMatchPattern> pattern(PartialMatchPattern::Create());
	pattern->SetWholeText(queryText);

	KillTaskCommand command;
	launcherapp::CommandQueryItemList items;
	ASSERT_TRUE(command.QueryCandidates(pattern.get(), items));

	bool foundProcess = false;
	for (size_t i = 0; i < items.GetItemCount(); ++i) {
		launcherapp::CommandQueryItem item;
		ASSERT_TRUE(items.GetItem(i, &item));
		if (item.mCommand->GetName() == _T("kill cmd.exe")) {
			foundProcess = true;
			break;
		}
	}
	EXPECT_TRUE(foundProcess);
}
