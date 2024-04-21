#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace filter {



class FilterExecutor
{
public:
	FilterExecutor();
	~FilterExecutor();

public:
	void ClearCandidates();
	void AddCandidates(const CString& item);
	void Execute(const CString& keyword, std::vector<CString>& result);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp


