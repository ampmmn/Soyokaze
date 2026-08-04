#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class CommandRepositoryListenerCountMetrics : public IResourceMetrics
{
public:
	CommandRepositoryListenerCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
