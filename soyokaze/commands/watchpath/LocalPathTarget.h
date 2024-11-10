#pragma once

#include "commands/watchpath/WatchTarget.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace watchpath {

class LocalPathTarget : public WatchTarget
{
public:
	LocalPathTarget(const CString& cmdName, const CString& message, const CString& path, UINT interval);
	virtual ~LocalPathTarget() ;

	bool IsUpdated() override;

	CString GetCommandName() override;
	CString GetTitle() override;
	CString GetDetail() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

