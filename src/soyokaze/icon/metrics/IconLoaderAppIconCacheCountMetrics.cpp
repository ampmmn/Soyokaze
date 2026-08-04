#include "pch.h"
#include "IconLoaderAppIconCacheCountMetrics.h"
#include "icon/IconLoader.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

IconLoaderAppIconCacheCountMetrics::IconLoaderAppIconCacheCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int IconLoaderAppIconCacheCountMetrics::GetOrder() const { return 140; }
std::string IconLoaderAppIconCacheCountMetrics::GetName() const { return "IconLoaderAppIconCacheCount"; }
std::string IconLoaderAppIconCacheCountMetrics::GetValue() const
{
	return std::to_string(IconLoader::Get()->GetCacheStatistics().appIconCacheCount);
}

namespace { static IconLoaderAppIconCacheCountMetrics metrics; }

}
