#include "pch.h"
#include "CandidateListHasErrorCommandMetrics.h"
#include "mainwindow/CandidateList.h"
#include "logger/ResourceUsageMonitor.h"

namespace logger {

CandidateListHasErrorCommandMetrics::CandidateListHasErrorCommandMetrics()
{
#ifndef SOYOKAZE_UNITTEST
	ResourceUsageMonitor::Get()->RegisterMetrics(this);
#endif
}

int CandidateListHasErrorCommandMetrics::GetOrder() const { return 420; }
std::string CandidateListHasErrorCommandMetrics::GetName() const { return "CandidateListHasErrorCommand"; }
std::string CandidateListHasErrorCommandMetrics::GetValue() const
{
	return std::to_string(CandidateList::GetStatistics().errorCommandCount);
}

namespace { static CandidateListHasErrorCommandMetrics metrics; }

}
