#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace activate_window {

class WindowList
{
public:
	WindowList();
	~WindowList();

public:
	void EnumWindowHandles(std::vector<HWND>& handles);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

}
}
}

