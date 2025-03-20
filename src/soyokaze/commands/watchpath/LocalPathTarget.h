#pragma once

#include "commands/watchpath/WatchTarget.h"
#include "commands/watchpath/PathWatcherItem.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace watchpath {

class LocalPathTarget : public WatchTarget
{
public:
	LocalPathTarget(const CString& cmdName, const PathWatcherItem& item);
	virtual ~LocalPathTarget() ;

	bool IsUpdated() override;

	CString GetCommandName() override;
	CString GetTitle() override;
	CString GetDetail() override;
	CString GetPath() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

