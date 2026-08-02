#include "pch.h"
#include "ResourceUsageMetrics.h"
#include "ResourceUsageMonitor.h"
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include "utility/ScopeExit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace logger {

WorkingSetMetrics::WorkingSetMetrics()
{
}

int WorkingSetMetrics::GetOrder() const
{
	return 10;
}

std::string WorkingSetMetrics::GetName() const
{
	return "WorkingSet";
}

std::string WorkingSetMetrics::GetValue() const
{
	PROCESS_MEMORY_COUNTERS_EX pmc = { sizeof(PROCESS_MEMORY_COUNTERS_EX) };
	if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc)) == FALSE) {
		return "0";
	}
	return std::to_string(pmc.WorkingSetSize);
}

PrivateBytesMetrics::PrivateBytesMetrics()
{
}

int PrivateBytesMetrics::GetOrder() const
{
	return 20;
}

std::string PrivateBytesMetrics::GetName() const
{
	return "PrivateBytes";
}

std::string PrivateBytesMetrics::GetValue() const
{
	PROCESS_MEMORY_COUNTERS_EX pmc = { sizeof(PROCESS_MEMORY_COUNTERS_EX) };
	if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc)) == FALSE) {
		return "0";
	}
	return std::to_string(pmc.PrivateUsage);
}

ThreadMetrics::ThreadMetrics()
{
}

int ThreadMetrics::GetOrder() const
{
	return 30;
}

std::string ThreadMetrics::GetName() const
{
	return "Threads";
}

std::string ThreadMetrics::GetValue() const
{
	// プロセス一覧から自プロセスを検索し、スレッド数を取得する。
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return "0";
	}
	::utility::ScopeExit guard([&]() { CloseHandle(snapshot); });

	PROCESSENTRY32 pe = { sizeof(pe) };
	if (Process32First(snapshot, &pe) == FALSE) {
		return "0";
	}

	const auto pid = GetCurrentProcessId();
	do {
		if (pe.th32ProcessID == pid) {
			return std::to_string(pe.cntThreads);
		}
	} while (Process32Next(snapshot, &pe));

	return "0";
}

GdiObjectsMetrics::GdiObjectsMetrics()
{
}

int GdiObjectsMetrics::GetOrder() const
{
	return 40;
}

std::string GdiObjectsMetrics::GetName() const
{
	return "GDI Objects";
}

std::string GdiObjectsMetrics::GetValue() const
{
	return std::to_string(GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS));
}

UserObjectsMetrics::UserObjectsMetrics()
{
}

int UserObjectsMetrics::GetOrder() const
{
	return 50;
}

std::string UserObjectsMetrics::GetName() const
{
	return "User Objects";
}

std::string UserObjectsMetrics::GetValue() const
{
	return std::to_string(GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS));
}

namespace {

static WorkingSetMetrics workingSetMetrics;
static PrivateBytesMetrics privateBytesMetrics;
static ThreadMetrics threadMetrics;
static GdiObjectsMetrics gdiObjectsMetrics;
static UserObjectsMetrics userObjectsMetrics;

bool EnsureDefaultResourceMetrics()
{
	// staticライブラリ使用時に、このファイルのstaticインスタンスを含む
	// オブジェクトファイルがリンクから除外されることを防ぎ、既定メトリクスを登録する。
	ResourceUsageMonitor::Get()->RegisterMetrics(&workingSetMetrics);
	ResourceUsageMonitor::Get()->RegisterMetrics(&privateBytesMetrics);
	ResourceUsageMonitor::Get()->RegisterMetrics(&threadMetrics);
	ResourceUsageMonitor::Get()->RegisterMetrics(&gdiObjectsMetrics);
	ResourceUsageMonitor::Get()->RegisterMetrics(&userObjectsMetrics);

	return true;
}

static bool metricsRegistered = EnsureDefaultResourceMetrics();

}

}
