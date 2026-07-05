#include "stdafx.h"
#include "gtest/gtest.h"
#include "logger/ResourceUsageMonitor.h"
#include <windows.h>
#include <string>
#include <filesystem>

using namespace logger;

class ResourceUsageMonitorTest : public ::testing::Test {
	protected:
		void SetUp() override {
			monitor = ResourceUsageMonitor::Get();
			ASSERT_TRUE(monitor->Initialize());
		}

		void TearDown() override {
			ASSERT_TRUE(monitor->Finalize());
		}

		ResourceUsageMonitor* monitor;
};

TEST_F(ResourceUsageMonitorTest, SingletonTest) {
	ResourceUsageMonitor* instance1 = ResourceUsageMonitor::Get();
	ResourceUsageMonitor* instance2 = ResourceUsageMonitor::Get();
	EXPECT_EQ(instance1, instance2);
}

TEST_F(ResourceUsageMonitorTest, InitializationTest) {
	EXPECT_TRUE(monitor->Initialize());
	EXPECT_TRUE(monitor->Finalize());
}

TEST_F(ResourceUsageMonitorTest, LogUsageTest) {
	// We can't test the actual file output without mocking, test the logic flow
	EXPECT_FALSE(monitor->LogUsage());  // Callable but won't log due to inactivity

	// Simulate enabling and logging
	EXPECT_FALSE(monitor->LogUsage());  // Should still do nothing due to timing

	std::wstring path;
	EXPECT_TRUE(ResourceUsageMonitor::GetLogFilePath(path));
	ASSERT_FALSE(path.empty());

	// テスト用にフォルダを作成する
	std::vector<wchar_t> dirPath(path.begin(), path.end());
	dirPath.push_back(L'\0');
	PathRemoveFileSpecW(dirPath.data());
	if (PathIsDirectory(dirPath.data()) == FALSE) {
		EXPECT_TRUE(std::filesystem::create_directories(dirPath.data()));
	}

	monitor->Enable();
	monitor->UpdateLastLoggedTimeStamp(0);
	EXPECT_TRUE(monitor->LogUsage());  // Should actually log now
}

TEST_F(ResourceUsageMonitorTest, ResourceRetrievalTest) {
	uint64_t workingSet, workingSetPeak;
	EXPECT_TRUE(ResourceUsageMonitor::GetWorkingSet(&workingSet, &workingSetPeak));
	EXPECT_GT(workingSet, 0);

	uint64_t privateBytes;
	EXPECT_TRUE(ResourceUsageMonitor::GetPrivateBytes(&privateBytes));
	EXPECT_GT(privateBytes, 0);

	uint32_t numOfThreads;
	EXPECT_TRUE(ResourceUsageMonitor::GetThreadUsage(&numOfThreads));
	EXPECT_GT(numOfThreads, 0UL);

	uint32_t gdiObjects;
	EXPECT_TRUE(ResourceUsageMonitor::GetGdiObjects(&gdiObjects));

	uint32_t userObjects;
	EXPECT_TRUE(ResourceUsageMonitor::GetUserObjects(&userObjects));
}

TEST_F(ResourceUsageMonitorTest, LogFormattingTest) {
	std::string header;
	EXPECT_TRUE(ResourceUsageMonitor::MakeHeader(header));
	EXPECT_EQ(header, "Time,PID,WorkingSet,PrivateBytes,Threads,GDI Objects, User Objects\n");

	std::string entry;
	EXPECT_TRUE(ResourceUsageMonitor::MakeLogEntry(entry));
	EXPECT_FALSE(entry.empty());
}

TEST_F(ResourceUsageMonitorTest, FilePathTest) {
	std::wstring path;
	EXPECT_TRUE(ResourceUsageMonitor::GetLogFilePath(path));
	EXPECT_FALSE(path.empty());
}
