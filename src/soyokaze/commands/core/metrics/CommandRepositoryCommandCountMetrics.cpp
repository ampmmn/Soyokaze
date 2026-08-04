#include "pch.h"
#include "CommandRepositoryCommandCountMetrics.h"
#include "commands/core/CommandRepository.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

CommandRepositoryCommandCountMetrics::CommandRepositoryCommandCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int CommandRepositoryCommandCountMetrics::GetOrder() const { return 300; }
std::string CommandRepositoryCommandCountMetrics::GetName() const { return "CommandRepositoryCommandCount"; }
std::string CommandRepositoryCommandCountMetrics::GetValue() const
{
	return std::to_string(launcherapp::core::CommandRepository::GetInstance()->GetCommandCount());
}

namespace { static CommandRepositoryCommandCountMetrics metrics; }

}
