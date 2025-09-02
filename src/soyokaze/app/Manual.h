#pragma once

#include <memory>

namespace launcherapp {
namespace app {

class Manual
{
	Manual();
	~Manual();

public:
	static Manual* GetInstance();

	bool Navigate(const char* pageId);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
