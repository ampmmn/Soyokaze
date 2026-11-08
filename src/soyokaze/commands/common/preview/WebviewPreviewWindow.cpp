#include "pch.h"
#include "WebviewPreviewWindow.h"
#include "SharedHwnd.h"
#include "control/webbrowser/InternalBrowser.h"

namespace launcherapp {
namespace commands {
namespace common {
namespace preview {

using InternalBrowser = soyokaze::control::webbrowser::InternalBrowser;

struct WebviewPreviewWindow::PImpl
{
	/**
	  WebView2プレビューウインドウを作成する
	  @return 作成できた場合はtrue
	*/
	bool Create()
	{
		SharedHwnd owner;
		if (mBrowser.Create(CWnd::FromHandle(owner.GetHwnd()), WS_POPUP, CRect(0, 0, 640, 480), 0) == FALSE) {
			return false;
		}
		if (mURL.IsEmpty() == FALSE) {
			mBrowser.Open(mURL);
		}
		return true;
	}

	InternalBrowser mBrowser;
	CString mURL;
};

WebviewPreviewWindow::WebviewPreviewWindow() : in(std::make_unique<PImpl>())
{
}

WebviewPreviewWindow::~WebviewPreviewWindow()
{
}

WebviewPreviewWindow* WebviewPreviewWindow::Get()
{
	static WebviewPreviewWindow window;
	return &window;
}

void WebviewPreviewWindow::SetFileUrl(const CString& url)
{
	in->mURL = url;
	if (in->mBrowser.GetSafeHwnd()) {
		in->mBrowser.Open(url);
	}
}

void WebviewPreviewWindow::Clear()
{
	in->mURL.Empty();
	if (in->mBrowser.GetSafeHwnd()) {
		in->mBrowser.ResetNavigation();
	}
}

void WebviewPreviewWindow::Enter()
{
	if (in->mBrowser.GetSafeHwnd() == nullptr && in->Create() == false) {
		return;
	}
	in->mBrowser.ShowWindow(SW_SHOW);
}

void WebviewPreviewWindow::Leave()
{
	if (in->mBrowser.GetSafeHwnd()) {
		in->mBrowser.ShowWindow(SW_HIDE);
	}
}

HWND WebviewPreviewWindow::GetHwnd()
{
	if (in->mBrowser.GetSafeHwnd() == nullptr && in->Create() == false) {
		return nullptr;
	}
	return in->mBrowser.GetSafeHwnd();
}

}
}
}
}
