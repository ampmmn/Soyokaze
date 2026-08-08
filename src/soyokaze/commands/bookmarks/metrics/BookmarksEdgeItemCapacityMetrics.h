#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class BookmarksEdgeItemCapacityMetrics : public IResourceMetrics
{
public:
	BookmarksEdgeItemCapacityMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
