#include "pch.h"
#include "PowerPointWindow.h"
#include "utility/ScopeAttachThreadInput.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"
#include "icon/IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace presentation {

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

struct PowerPointWindow::PImpl
{
	HWND mHwnd{nullptr};
};


PowerPointWindow::PowerPointWindow() : in(new PImpl)
{
	in->mHwnd = nullptr;
}

PowerPointWindow::~PowerPointWindow()
{
}

bool PowerPointWindow::Activate(bool isShowMaximize)
{
	HWND hwnd = in->mHwnd;

	if (IsWindow(hwnd) == FALSE) {
		return false;
	}

	ScopeAttachThreadInput scope;

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (isShowMaximize && (style & WS_MAXIMIZE) == 0) {
			// Ctrlキーが押されていたら最大化表示する
			PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	else if (style & WS_MINIMIZE) {
		// 最小化されていたら元に戻す
		PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	}

	SetForegroundWindow(hwnd);

	return true;
}

bool PowerPointWindow::GetAcitveWindow(std::unique_ptr<PowerPointWindow>& ptr)
{
	// PowerPointアプリのウインドウハンドルを取得する
	HWND hwnd = nullptr;
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	if (proxy->GetActivePowerPointWindow(hwnd) == false || IsWindow(hwnd) == FALSE) {
		return false;
	}

	std::unique_ptr<PowerPointWindow> obj(new PowerPointWindow);
	obj->in->mHwnd = hwnd;
	ptr.reset(obj.release());

	return true;
}

bool PowerPointWindow::GoToSlide(int16_t pageIndex)
{
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	return proxy->GoToSlide(pageIndex); 
}

HICON PowerPointWindow::ResolveIcon()
{
	// 既に取得済の場合はそのアイコンを返す
	static HICON sIcon = nullptr;
	if (sIcon) {
		return sIcon;
	}

	auto iconLoader = IconLoader::Get();

	// PowerPointアプリのウインドウハンとるを取得する
	HWND hwnd = nullptr;
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	if (proxy->GetActivePowerPointWindow(hwnd) == false || IsWindow(hwnd) == FALSE) {
		return iconLoader->LoadUnknownIcon();
	}

	// ウインドウハンドルからアイコンを取得する
	sIcon = iconLoader->LoadIconFromHwnd(hwnd);
	return sIcon;
}

}}} // end of namespace launcherapp::commands::presentation
