#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class CandidateListHasErrorCommandMetrics : public IResourceMetrics
{
public:
	CandidateListHasErrorCommandMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
