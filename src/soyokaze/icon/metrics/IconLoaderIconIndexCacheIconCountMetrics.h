#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class IconLoaderIconIndexCacheIconCountMetrics : public IResourceMetrics
{
public:
	IconLoaderIconIndexCacheIconCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
