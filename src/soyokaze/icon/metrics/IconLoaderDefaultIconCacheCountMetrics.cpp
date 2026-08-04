#include "pch.h"
#include "IconLoaderDefaultIconCacheCountMetrics.h"
#include "icon/IconLoader.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

IconLoaderDefaultIconCacheCountMetrics::IconLoaderDefaultIconCacheCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int IconLoaderDefaultIconCacheCountMetrics::GetOrder() const { return 120; }
std::string IconLoaderDefaultIconCacheCountMetrics::GetName() const { return "IconLoaderDefaultIconCacheCount"; }
std::string IconLoaderDefaultIconCacheCountMetrics::GetValue() const
{
	return std::to_string(IconLoader::Get()->GetCacheStatistics().defaultIconCacheCount);
}

namespace { static IconLoaderDefaultIconCacheCountMetrics metrics; }

}
