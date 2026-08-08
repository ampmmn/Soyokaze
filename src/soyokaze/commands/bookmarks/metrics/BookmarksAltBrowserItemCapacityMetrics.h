#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class BookmarksAltBrowserItemCapacityMetrics : public IResourceMetrics
{
public:
	BookmarksAltBrowserItemCapacityMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
