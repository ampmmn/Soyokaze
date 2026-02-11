#pragma once

#include <windows.h>
#include <memory>
#include <vector>
#include <string>
#include <UIAutomation.h>

#include "commands/uiautomation/WindowUIElement.h"
#include "utility/RefPtr.h"

namespace launcherapp { namespace commands { namespace uiautomation {

class WindowUIElements
{
public:
	using UIElementList = std::vector<RefPtr<UIElement> >;

public:
	WindowUIElements(HWND hwnd);
	~WindowUIElements();

	bool FetchElements(UIElementList& items);
	bool FetchTabItemElements(UIElementList& items);
	bool FetchWin32MenuItems(UIElementList& items);

	void Dump();

protected:
	bool FetchElements(UIElementList& items, int findType);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}} // end of namespace launcherapp::commands::uiautomation

