#include "pch.h"
#include "WindowList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace activate_window {

struct WindowList::PImpl
{
	bool mHasHandles{false};
	std::vector<HWND> mHandles;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

WindowList::WindowList() : in(new PImpl)
{
}

WindowList::~WindowList()
{
}

void WindowList::Clear()
{
	in->mHasHandles = false;
	in->mHandles.clear();
}

static bool IsTopLevelWindow(HWND hwnd)
{
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	LONG_PTR styleRequired = WS_VISIBLE;

	if ((style & styleRequired) != styleRequired) {
		// 非表示のウインドウは対象外
		return false;
	}

	if (style & WS_DISABLED) {
		// 無効化されているウインドウは除外
		return false;
	}

	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == GetCurrentProcessId()) {
		// 自分自身のウインドウは除外
		return false;
	}

	TCHAR c[4] = {};
	GetWindowText(hwnd, c, 4);
	if (c[0] == _T('\0')) {
		// キャプションが空っぽの場合は除外
		return false;
	}

	LONG_PTR styleEx = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	if (styleEx & (WS_EX_NOACTIVATE)) {
		return false;
	}

	CRect rc;
	GetWindowRect(hwnd, rc);
	if (rc.Width() == 0 || rc.Height() == 0) {
		// サイズ的に見えないものは除外
		return false;
	}

	return true;
}

void WindowList::EnumWindowHandles(std::vector<HWND>& handles)
{
	if (in->mHasHandles) {
		handles = in->mHandles;
		return;
	}


	std::vector<HWND> tmpHandles;

	auto hwnd = GetTopWindow(nullptr);
	while(hwnd) {
		if (IsTopLevelWindow(hwnd)) {
			tmpHandles.push_back(hwnd);
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}

	in->mHandles.swap(tmpHandles);
	in->mHasHandles = true;

	handles = in->mHandles;
}


}}}

