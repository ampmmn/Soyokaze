#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class IconLoaderFileExtIconCacheCountMetrics : public IResourceMetrics
{
public:
	IconLoaderFileExtIconCacheCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
