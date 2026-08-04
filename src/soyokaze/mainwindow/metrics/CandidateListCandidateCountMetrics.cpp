#include "pch.h"
#include "CandidateListCandidateCountMetrics.h"
#include "mainwindow/CandidateList.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

CandidateListCandidateCountMetrics::CandidateListCandidateCountMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int CandidateListCandidateCountMetrics::GetOrder() const { return 400; }
std::string CandidateListCandidateCountMetrics::GetName() const { return "CandidateListCandidateCount"; }
std::string CandidateListCandidateCountMetrics::GetValue() const
{
	return std::to_string(CandidateList::GetStatistics().candidateCount);
}

namespace { static CandidateListCandidateCountMetrics metrics; }

}
