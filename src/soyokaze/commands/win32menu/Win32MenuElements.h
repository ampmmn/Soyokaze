#pragma once

#include <windows.h>
#include <memory>
#include <vector>
#include <string>

#include "commands/win32menu/Win32MenuElement.h"
#include "utility/RefPtr.h"

namespace launcherapp { namespace commands { namespace win32menu {

class Win32MenuElements
{
public:
	using Win32MenuElementList = std::vector<Win32MenuItemElement>;

public:
	Win32MenuElements(HWND hwnd);
	~Win32MenuElements();

	bool FetchWin32MenuItems(Win32MenuElementList& items);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}} // end of namespace launcherapp::commands::win32menu

