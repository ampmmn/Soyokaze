#pragma once

#include <windows.h>
#include <memory>
#include <vector>
#include <string>

#include "commands/win32menu/Win32MenuElement.h"

namespace launcherapp { namespace commands { namespace win32menu {

class Win32MenuElements
{
public:
	using Win32MenuElementList = std::vector<Win32MenuItemElement>;

public:
	Win32MenuElements();
	~Win32MenuElements();

	bool FetchWin32MenuItems(HWND hwnd, Win32MenuElementList& items);
};

}}} // end of namespace launcherapp::commands::win32menu

