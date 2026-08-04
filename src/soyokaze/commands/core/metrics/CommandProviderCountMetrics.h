#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class CommandProviderCountMetrics : public IResourceMetrics
{
public:
	CommandProviderCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
