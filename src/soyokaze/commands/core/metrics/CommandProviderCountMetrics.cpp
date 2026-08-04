#include "pch.h"
#include "CommandProviderCountMetrics.h"
#include "commands/core/CommandProviderRepository.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

CommandProviderCountMetrics::CommandProviderCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int CommandProviderCountMetrics::GetOrder() const { return 200; }
std::string CommandProviderCountMetrics::GetName() const { return "CommandProviderCount"; }
std::string CommandProviderCountMetrics::GetValue() const
{
	return std::to_string(launcherapp::core::CommandProviderRepository::GetInstance()->GetProviderCount());
}

namespace { static CommandProviderCountMetrics metrics; }

}
