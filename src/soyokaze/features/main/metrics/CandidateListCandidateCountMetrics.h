#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class CandidateListCandidateCountMetrics : public IResourceMetrics
{
public:
	CandidateListCandidateCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
