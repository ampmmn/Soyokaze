#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class CommandRepositoryCommandCountMetrics : public IResourceMetrics
{
public:
	CommandRepositoryCommandCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
