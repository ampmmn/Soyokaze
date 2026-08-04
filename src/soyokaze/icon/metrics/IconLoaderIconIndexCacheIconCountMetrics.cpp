#include "pch.h"
#include "IconLoaderIconIndexCacheIconCountMetrics.h"
#include "icon/IconLoader.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

IconLoaderIconIndexCacheIconCountMetrics::IconLoaderIconIndexCacheIconCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int IconLoaderIconIndexCacheIconCountMetrics::GetOrder() const { return 110; }
std::string IconLoaderIconIndexCacheIconCountMetrics::GetName() const { return "IconLoaderIconIndexCacheIconCount"; }
std::string IconLoaderIconIndexCacheIconCountMetrics::GetValue() const
{
	return std::to_string(IconLoader::Get()->GetCacheStatistics().iconIndexCacheIconCount);
}

namespace { static IconLoaderIconIndexCacheIconCountMetrics metrics; }

}
