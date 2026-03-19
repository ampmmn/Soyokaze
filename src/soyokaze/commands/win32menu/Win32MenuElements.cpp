#include "pch.h"
#include "Win32MenuElements.h"
#include "commands/win32menu/UIElementAliasMap.h"
#include "utility/Path.h"
#include <list>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <utility>

using json = nlohmann::json;

namespace launcherapp { namespace commands { namespace win32menu {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


Win32MenuElements::Win32MenuElements()
{
}

Win32MenuElements::~Win32MenuElements()
{
}


bool Win32MenuElements::FetchWin32MenuItems(HWND hwnd, Win32MenuElementList& items)
{
	if (IsWindow(hwnd) == FALSE) {
		// 対象ウインドウが無効
		return false;
	}

	// ウインドウのメインメニューを取得する
	HMENU hTopMenu = GetMenu(hwnd);
	if (hTopMenu == nullptr) {
		// メニュー持ってない
		return false;
	}

	std::set<UINT> commandIds;

	std::vector<std::pair<HMENU, CString> > stk;
	stk.push_back(std::make_pair(hTopMenu, _T("")));

	while( stk.empty() == false) {

		auto item = stk.back();
		stk.pop_back();

		HMENU h = item.first;
		CString parentLabel = item.second;

		int count = GetMenuItemCount(h);
		for (int i = 0; i < count; ++i) {
			TCHAR text[256] = { _T('\0') };

			MENUITEMINFO info{ sizeof(MENUITEMINFO), MIIM_STRING | MIIM_STATE | MIIM_FTYPE };
			info.cch = 256;
			info.dwTypeData = text;

			GetMenuItemInfo(h, i, TRUE, &info);

			if (info.fState & MFS_DISABLED) {
				continue;
			}
			if ((info.fType & MFT_OWNERDRAW) || text[0] == _T('\0')) {
				// オーナードローの項目は、名前が取れないことがあるので、インデックスを名前にする
				_stprintf_s(text, _T("(%d)"), i + 1);
			}

			CString text_;
			if (parentLabel.IsEmpty() == FALSE) {
				text_ = parentLabel + _T(" > ") + text;
			}
			else {
				text_ = text;
			}
			text_.Replace(_T("\t"), _T(" "));
			text_.Replace(_T("&"), _T(""));

			auto subMenu = GetSubMenu(h, i);
			if (subMenu == nullptr) {
				UINT commandId = GetMenuItemID(h, i);
				items.push_back(Win32MenuItemElement(hwnd, commandId, text_));
				commandIds.insert(commandId);
			}
			else {
				// 子要素を探す
				stk.push_back(std::make_pair(subMenu, text_));
			}
		}
	}

	// キャッシュ(uielement-menu-alias.json)にある項目を取得
	std::vector<std::pair<UINT,CString> > cachedMenuElements;

	auto aliasMap = UIElementAliasMap::GetInstance();
	TCHAR clsName[256];
	GetClassName(hwnd, clsName, 256);
	aliasMap->EnumElements(clsName, cachedMenuElements);

	for (auto& elem : cachedMenuElements) {
		auto& commandId = elem.first;
		auto& name = elem.second;
		if (commandIds.count(commandId) != 0) {
			continue;
		}
		items.push_back(Win32MenuItemElement(hwnd, commandId, name));
	}

	return true;
}


}}} // end of namespace launcherapp::commands::win32menu
