#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class IconLoaderAppIconCacheCountMetrics : public IResourceMetrics
{
public:
	IconLoaderAppIconCacheCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
