#include "pch.h"
#include "IconLoaderIconIndexCachePathCountMetrics.h"
#include "icon/IconLoader.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

IconLoaderIconIndexCachePathCountMetrics::IconLoaderIconIndexCachePathCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int IconLoaderIconIndexCachePathCountMetrics::GetOrder() const { return 100; }
std::string IconLoaderIconIndexCachePathCountMetrics::GetName() const { return "IconLoaderIconIndexCachePathCount"; }
std::string IconLoaderIconIndexCachePathCountMetrics::GetValue() const
{
	return std::to_string(IconLoader::Get()->GetCacheStatistics().iconIndexCachePathCount);
}

namespace { static IconLoaderIconIndexCachePathCountMetrics metrics; }

}
