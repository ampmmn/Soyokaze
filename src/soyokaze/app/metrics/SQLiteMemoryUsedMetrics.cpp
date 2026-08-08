#include "pch.h"
#include "SQLiteMemoryUsedMetrics.h"
#include "logger/ResourceUsageMonitor.h"
#include "utility/SQLite3Wrapper.h"

namespace logger {

SQLiteMemoryUsedMetrics::SQLiteMemoryUsedMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int SQLiteMemoryUsedMetrics::GetOrder() const
{
	return 60;
}

std::string SQLiteMemoryUsedMetrics::GetName() const
{
	return "SQLiteMemoryUsed";
}

std::string SQLiteMemoryUsedMetrics::GetValue() const
{
	return std::to_string(launcherapp::utility::SQLite3Wrapper::Get()->MemoryUsed());
}

namespace {
	static SQLiteMemoryUsedMetrics metrics;
}

}
