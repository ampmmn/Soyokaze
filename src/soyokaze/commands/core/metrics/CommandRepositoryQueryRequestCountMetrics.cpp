#include "pch.h"
#include "CommandRepositoryQueryRequestCountMetrics.h"
#include "commands/core/CommandRepository.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

CommandRepositoryQueryRequestCountMetrics::CommandRepositoryQueryRequestCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int CommandRepositoryQueryRequestCountMetrics::GetOrder() const { return 310; }
std::string CommandRepositoryQueryRequestCountMetrics::GetName() const { return "CommandRepositoryQueryRequestCount"; }
std::string CommandRepositoryQueryRequestCountMetrics::GetValue() const
{
	return std::to_string(launcherapp::core::CommandRepository::GetInstance()->GetQueryRequestCount());
}

namespace { static CommandRepositoryQueryRequestCountMetrics metrics; }

}
