#pragma once

#include "commands/uwp/UWPApplicationItem.h"

#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace uwp {

class UWPApplications
{
public:
	UWPApplications();
	~UWPApplications();

public:
	bool GetApplications(std::vector<ItemPtr>& items);


private:
	struct PImpl;
	std::unique_ptr<PImpl> in;


};


} // end of namespace uwp
} // end of namespace commands
} // end of namespace launcherapp

