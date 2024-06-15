#pragma once

#include <vector>

namespace launcherapp {
namespace commands {
namespace filter {


class FilterResult
{
public:
	FilterResult() = default;
	FilterResult(int level, const CString& name) : 
		mMatchLevel(level), mDisplayName(name)
	{
	}
	FilterResult(const FilterResult&) = default;


	int mMatchLevel;
	CString mDisplayName;
};

using FilterResultList = std::vector<FilterResult>;


} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp


