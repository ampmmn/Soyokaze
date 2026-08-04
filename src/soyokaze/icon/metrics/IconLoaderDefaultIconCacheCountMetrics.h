#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class IconLoaderDefaultIconCacheCountMetrics : public IResourceMetrics
{
public:
	IconLoaderDefaultIconCacheCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
