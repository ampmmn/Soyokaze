#pragma once

#include <memory>
#include "commands/filter/FilterResult.h"

namespace launcherapp {
namespace commands {
namespace filter {

class CommandParam;

class FilterExecutor
{
public:
	FilterExecutor();
	~FilterExecutor();

public:
	void LoadCandidates(const CommandParam& param);
	bool IsLoaded();

	void ClearCandidates();
	void AddCandidates(const CString& item);
	size_t GetCandidatesCount();

	void Query(const CString& keyword, FilterResultList& result);

	LONG AddRef();
	LONG Release();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp


