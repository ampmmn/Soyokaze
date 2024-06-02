#pragma once

#include "commands/mmc/MMCSnapin.h"
#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace mmc {

class MMCSnapins
{
public:
	MMCSnapins();
	~MMCSnapins();

	void GetSnapins(std::vector<MMCSnapin>& items);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace mmc
} // end of namespace commands
} // end of namespace launcherapp

