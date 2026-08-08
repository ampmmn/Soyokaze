#include "pch.h"
#include "BookmarksEdgeItemCapacityMetrics.h"
#include "commands/bookmarks/BookmarkCommand.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

BookmarksEdgeItemCapacityMetrics::BookmarksEdgeItemCapacityMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int BookmarksEdgeItemCapacityMetrics::GetOrder() const { return 230; }
std::string BookmarksEdgeItemCapacityMetrics::GetName() const { return "BookmarksEdgeItemCapacity"; }
std::string BookmarksEdgeItemCapacityMetrics::GetValue() const
{
	return std::to_string(launcherapp::commands::bookmarks::BookmarkCommand::GetStatistics().edgeItemCapacity);
}

namespace { static BookmarksEdgeItemCapacityMetrics metrics; }

}
