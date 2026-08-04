#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class IconLoaderIconIndexCachePathCountMetrics : public IResourceMetrics
{
public:
	IconLoaderIconIndexCachePathCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
