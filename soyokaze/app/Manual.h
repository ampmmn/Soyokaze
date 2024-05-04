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

	bool Navigate(const CString& pageId);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
