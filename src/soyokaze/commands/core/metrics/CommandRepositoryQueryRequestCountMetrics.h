#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class CommandRepositoryQueryRequestCountMetrics : public IResourceMetrics
{
public:
	CommandRepositoryQueryRequestCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
