#pragma once

#include <UIAutomation.h>

namespace launcherapp { namespace commands { namespace win32menu {

class Win32MenuItemElement
{
public:
	Win32MenuItemElement();
	Win32MenuItemElement(HWND hwnd, UINT id, const CString& name);
	~Win32MenuItemElement();

	HWND GetHwnd();
	CString GetName();
	CRect GetRect();
	bool Click();
	bool Focus();
	bool CanFocus();

	HWND mHwnd{nullptr};
	UINT mMenuId{0};
	CString mName;
};

}}} // end of namespace launcherapp::commands::win32menu

