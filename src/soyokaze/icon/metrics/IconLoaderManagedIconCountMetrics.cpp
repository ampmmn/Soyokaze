#include "pch.h"
#include "IconLoaderManagedIconCountMetrics.h"
#include "icon/IconLoader.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

IconLoaderManagedIconCountMetrics::IconLoaderManagedIconCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int IconLoaderManagedIconCountMetrics::GetOrder() const { return 150; }
std::string IconLoaderManagedIconCountMetrics::GetName() const { return "IconLoaderManagedIconCount"; }
std::string IconLoaderManagedIconCountMetrics::GetValue() const
{
	return std::to_string(IconLoader::Get()->GetCacheStatistics().managedIconCount);
}

namespace { static IconLoaderManagedIconCountMetrics metrics; }

}
