#pragma once

#include "logger/IResourceMetrics.h"

namespace logger {

class CandidateListListenerCountMetrics : public IResourceMetrics
{
public:
	CandidateListListenerCountMetrics();

	int GetOrder() const override;
	std::string GetName() const override;
	std::string GetValue() const override;
};

}
