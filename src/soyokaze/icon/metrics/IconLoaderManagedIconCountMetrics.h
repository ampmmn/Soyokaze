#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class IconLoaderManagedIconCountMetrics : public IResourceMetrics
{
public:
	IconLoaderManagedIconCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
