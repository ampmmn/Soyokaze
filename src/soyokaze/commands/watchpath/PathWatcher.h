#pragma once

#include "commands/watchpath/PathWatcherItem.h"

namespace launcherapp {
namespace commands {
namespace watchpath {

class PathWatcher
{
public:
	using ITEM = PathWatcherItem;

private:
	PathWatcher();
	~PathWatcher();

public:
	static PathWatcher* Get();

	void RegisterPath(const CString& cmdName, const ITEM& item);
	void UnregisterPath(const CString& cmdName);
	
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

