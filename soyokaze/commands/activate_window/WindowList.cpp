#include "pch.h"
#include "WindowList.h"
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_window {

// 切り替え候補ウインドウの一覧を再利用する間隔
static const int HWNDUPDATE_INTERVAL = 5000;

struct WindowList::PImpl
{
	PImpl() : mEvent(TRUE, TRUE)
	{}

	uint64_t mLastHwndUpdate = 0;
	std::mutex mMutex;
	std::vector<HWND> mHandles;
	CEvent mEvent;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

WindowList::WindowList() : in(new PImpl)
{
}

WindowList::~WindowList()
{
	WaitForSingleObject(in->mEvent, 500);
}

static BOOL CALLBACK OnEnumWindows(HWND hwnd, LPARAM lParam)
{
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	LONG_PTR styleRequired = (WS_VISIBLE | WS_CAPTION);

	if ((style & styleRequired) != styleRequired) {
		// 非表示のウインドウと、タイトルを持たないウインドウは対象外
		return TRUE;
	}
	if (style & WS_DISABLED) {
		// 無効化されているウインドウは除外
		return TRUE;
	}
	TCHAR c[4] = {};
	GetWindowText(hwnd, c, 4);
	if (c[0] == _T('\0')) {
		// キャプションが空っぽの場合は除外
		return TRUE;
	}
	CRect rc;
	GetWindowRect(hwnd, rc);
	if (rc.Width() == 0 || rc.Height() == 0) {
		// サイズ的に見えないものは除外
		return TRUE;
	}

	auto handles = (std::vector<HWND>*)lParam;
	handles->push_back(hwnd);

	return TRUE;
}

void WindowList::EnumWindowHandles(std::vector<HWND>& handles)
{
	// 一定時間内の再実行の場合は過去の結果を再利用する
	if (GetTickCount64() - in->mLastHwndUpdate < HWNDUPDATE_INTERVAL) {
		std::lock_guard<std::mutex> lock(in->mMutex);
		handles = in->mHandles;
		return ;
	}

	in->mEvent.ResetEvent();

	std::thread th([&]() {
		std::vector<HWND> handles;
		EnumWindows(OnEnumWindows, (LPARAM)&handles);
		std::lock_guard<std::mutex> lock(in->mMutex);
		in->mHandles.swap(handles);
		in->mEvent.SetEvent();
		in->mLastHwndUpdate = GetTickCount64();
	});
	th.detach();

}


}
}
}

