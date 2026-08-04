#include "pch.h"
#include "CommandRepositoryListenerCountMetrics.h"
#include "commands/core/CommandRepository.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

CommandRepositoryListenerCountMetrics::CommandRepositoryListenerCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int CommandRepositoryListenerCountMetrics::GetOrder() const { return 320; }
std::string CommandRepositoryListenerCountMetrics::GetName() const { return "CommandRepositoryListenerCount"; }
std::string CommandRepositoryListenerCountMetrics::GetValue() const
{
	return std::to_string(launcherapp::core::CommandRepository::GetInstance()->GetListenerCount());
}

namespace { static CommandRepositoryListenerCountMetrics metrics; }

}
