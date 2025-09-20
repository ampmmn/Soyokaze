#pragma once

#include <memory>
#include "commands/filter/FilterResult.h"

namespace launcherapp {
namespace commands {
namespace filter {

class CommandParam;

// $B%-%c%s%;%k$5$l$?$+$I$&$+$N%U%i%04IM}(B
class CancellationToken
{
public:
	virtual ~CancellationToken() = default;
	// $B%-%c%s%;%k$,H/@8$7$?$+(B
	virtual bool IsCancellationRequested() = 0;

};


class FilterExecutor
{
public:
	FilterExecutor();
	~FilterExecutor();

public:
	void SetCancellationToken(CancellationToken* cancelToken);
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


