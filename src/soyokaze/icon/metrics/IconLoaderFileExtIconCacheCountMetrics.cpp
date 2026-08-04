#include "pch.h"
#include "IconLoaderFileExtIconCacheCountMetrics.h"
#include "icon/IconLoader.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

IconLoaderFileExtIconCacheCountMetrics::IconLoaderFileExtIconCacheCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int IconLoaderFileExtIconCacheCountMetrics::GetOrder() const { return 130; }
std::string IconLoaderFileExtIconCacheCountMetrics::GetName() const { return "IconLoaderFileExtIconCacheCount"; }
std::string IconLoaderFileExtIconCacheCountMetrics::GetValue() const
{
	return std::to_string(IconLoader::Get()->GetCacheStatistics().fileExtIconCacheCount);
}

namespace { static IconLoaderFileExtIconCacheCountMetrics metrics; }

}
