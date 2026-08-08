#include "pch.h"
#include "BookmarksAltBrowserItemCapacityMetrics.h"
#include "commands/bookmarks/BookmarkCommand.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

BookmarksAltBrowserItemCapacityMetrics::BookmarksAltBrowserItemCapacityMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int BookmarksAltBrowserItemCapacityMetrics::GetOrder() const { return 240; }
std::string BookmarksAltBrowserItemCapacityMetrics::GetName() const { return "BookmarksAltBrowserItemCapacity"; }
std::string BookmarksAltBrowserItemCapacityMetrics::GetValue() const
{
	return std::to_string(launcherapp::commands::bookmarks::BookmarkCommand::GetStatistics().alternativeBookmarkItemCapacity);
}

namespace { static BookmarksAltBrowserItemCapacityMetrics metrics; }

}
