#include "pch.h"
#include "Win32MenuElement.h"
#include "commands/win32menu/UIElementAliasMap.h"
#include "utility/ScopeAttachThreadInput.h"
#include "SharedHwnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace win32menu {

Win32MenuItemElement::Win32MenuItemElement()
{
}

Win32MenuItemElement::Win32MenuItemElement(HWND hwnd, UINT id, const CString& name) : 
	mHwnd(hwnd), mMenuId(id), mName(name)
{
}

Win32MenuItemElement::~Win32MenuItemElement()
{
}

HWND Win32MenuItemElement::GetHwnd()
{
	return mHwnd; 
}

CString Win32MenuItemElement::GetName()
{
	auto nameMap = UIElementAliasMap::GetInstance();

	TCHAR clsName[256];
	GetClassName(mHwnd, clsName, 256);

	CString alias;
	if (nameMap->GetAlias(clsName, mName, mMenuId, alias)) {
		return alias;
	}
	else {
		return mName;
	}
}

CRect Win32MenuItemElement::GetRect()
{
	return CRect(0,0,0,0);
}

bool Win32MenuItemElement::Click()
{
	if (IsWindow(mHwnd) == FALSE) {
		return false;
	}

	// 親ウインドウを前面に出す
	ScopeAttachThreadInput scope;
	SetForegroundWindow(mHwnd);

	// メニューを選択する操作
	SendMessage(mHwnd, WM_COMMAND, MAKEWPARAM(mMenuId, 0), 0);

	return true;
}

bool Win32MenuItemElement::Focus()
{
	return false;
}

bool Win32MenuItemElement::CanFocus()
{
	return false;
}

}}} // end of namespace launcherapp::commands::win32menu

