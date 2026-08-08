#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class SQLiteMemoryUsedMetrics : public IResourceMetrics
{
public:
	SQLiteMemoryUsedMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
