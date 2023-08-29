#pragma once

#include "commands/uwp/UWPApplicationItem.h"

#include <memory>
#include <vector>

namespace soyokaze {
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
	void EnumApplications(std::vector<ItemPtr>& items);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;


};


} // end of namespace uwp
} // end of namespace commands
} // end of namespace soyokaze

