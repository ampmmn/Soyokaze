#pragma once

#include <windows.h>
#include <memory>
#include <vector>
#include <string>

namespace launcherapp { namespace commands { namespace uiautomation {

class WindowUIElement
{
public:
	std::wstring mName;
	RECT mRect;
};

class WindowUIElements
{
public:
	WindowUIElements(HWND hwnd);
	~WindowUIElements();

	bool FetchElements(std::vector<WindowUIElement>& items);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}} // end of namespace launcherapp::commands::uiautomation

