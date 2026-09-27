#include "pch.h"
#include "CandidateListListenerCountMetrics.h"
#include "features/main/CandidateList.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

CandidateListListenerCountMetrics::CandidateListListenerCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int CandidateListListenerCountMetrics::GetOrder() const { return 410; }
std::string CandidateListListenerCountMetrics::GetName() const { return "CandidateListListenerCount"; }
std::string CandidateListListenerCountMetrics::GetValue() const
{
	return std::to_string(CandidateList::GetStatistics().listenerCount);
}

namespace { static CandidateListListenerCountMetrics metrics; }

}
