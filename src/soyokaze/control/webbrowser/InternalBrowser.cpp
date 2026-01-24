#include "pch.h"
#include "InternalBrowser.h"
#include "control/WindowPosition.h"
#include <winrt/base.h>
#include <wrl.h>
#include <wil/com.h>
#include <mutex>
#include <deque>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::WRL;

namespace soyokaze {
namespace control {
namespace webbrowser {


static bool sIsWndClassRegistered = false;

static bool RegigsterWindowClass()
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = InternalBrowser::WindowProc;
	wc.hInstance     = GetModuleHandle(nullptr);
	wc.hIcon         = nullptr;
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"LauncherInernalBrowser";

	if (RegisterClassEx(&wc) == FALSE) {
		spdlog::error("Failed to regsiter guidectrl class.");
		return false;
	}
	return true;
}

// InternalBrowser

IMPLEMENT_DYNAMIC(InternalBrowser, CWnd)

struct InternalBrowser::PImpl 
{
	void AddOpenRequest(const CString& url) {
		std::lock_guard<std::mutex> lock(mMutex);
		mRequest.push_back(url);
		if (IsWindow(mSelfWindow)) {
			::PostMessage(mSelfWindow, WM_APP+1, 0, 0);
		}
	}
	bool PopRequest(CString& url) {
		std::lock_guard<std::mutex> lock(mMutex);
		if (mRequest.empty()) {
			return false;
		}
		url = mRequest.back();
		return true;
	}

	wil::com_ptr<ICoreWebView2Controller> mWebViewCtrl;
	wil::com_ptr<ICoreWebView2> mWebView;

	CStringW mHostWindowClassName;
	HWND mSelfWindow{nullptr};

	std::mutex mMutex;
	std::deque<CString> mRequest;
	std::unique_ptr<WindowPosition> mWindowPositionPtr;
};


InternalBrowser::InternalBrowser() : in(std::make_unique<PImpl>())
{
}

InternalBrowser::~InternalBrowser()
{
}

void InternalBrowser::SetHostWindowClass(LPCWSTR clsName)
{
	in->mHostWindowClassName = clsName;
}

void InternalBrowser::EnableSaveWindowPosition(LPCTSTR settingName)
{
	in->mWindowPositionPtr = std::make_unique<WindowPosition>(settingName);
}

BOOL InternalBrowser::Create(CWnd* pParentWnd, int style, const RECT& rect, UINT nID)
{
	if (in->mHostWindowClassName.IsEmpty() && sIsWndClassRegistered == false) {
		RegigsterWindowClass();
		sIsWndClassRegistered = true;
	}

	CStringW clsName(in->mHostWindowClassName.IsEmpty() == FALSE ? in->mHostWindowClassName : _T("LauncherInernalBrowser"));

	if (CWnd::CreateEx(0, clsName, _T("InternalBrowser"), style, rect, pParentWnd, nID) == FALSE) {
		return FALSE;
	}
	in->mSelfWindow = GetSafeHwnd();
	SetWindowLongPtr(in->mSelfWindow , GWLP_USERDATA, (LONG_PTR)this);
	::EnableWindow(in->mSelfWindow, TRUE);

	// 必要に応じてウインドウ位置を復元
	if (in->mWindowPositionPtr.get()) {
		in->mWindowPositionPtr->Restore(in->mSelfWindow);
	}

	InitializeWebview();

	return TRUE;
}

void InternalBrowser::Open(const CString& url)
{
	in->AddOpenRequest(url);
}

void InternalBrowser::GoBack()
{
	if (in->mWebView) {
		in->mWebView->GoBack();
	}
}

void InternalBrowser::GoForward()
{
	if (in->mWebView) {
		in->mWebView->GoForward();
	}
}

void InternalBrowser::Reload()
{
	if (in->mWebView) {
		in->mWebView->Reload();
	}
}

BEGIN_MESSAGE_MAP(InternalBrowser, CWnd)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()

void InternalBrowser::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();
//	InitializeWebview();
}

void InternalBrowser::InitializeWebview()
{
	auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();

	auto h = GetSafeHwnd();

	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, options.Get(),
			Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
				[this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

				if (FAILED(result)) {
					return result;
				}

				env->CreateCoreWebView2Controller(GetSafeHwnd(), Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
							[this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
					if (FAILED(result)) {
						return result;
					}

					in->mWebViewCtrl = controller;
					in->mWebViewCtrl->get_CoreWebView2(&in->mWebView);

					RECT bounds;
					GetClientRect(&bounds);
					in->mWebViewCtrl->put_Bounds(bounds);
					// 開発者ツールを無効化する
					wil::com_ptr<ICoreWebView2Settings> settings;
					in->mWebView->get_Settings(&settings);
					settings->put_AreDevToolsEnabled(FALSE);

					::PostMessage(in->mSelfWindow, WM_APP+1, 0, 0);
					return S_OK;
				}).Get());
		return S_OK;
	}).Get());
}

// InternalBrowser メッセージ ハンドラー

void InternalBrowser::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// ウインドウ全体にwebviewを表示
	if (in->mWebViewCtrl) {
		RECT bounds;
		GetClientRect(&bounds);
		in->mWebViewCtrl->put_Bounds(bounds);
	}
}

void InternalBrowser::OnClose()
{
	if (in->mWindowPositionPtr.get()) {
		in->mWindowPositionPtr->Update(GetSafeHwnd());
		in->mWindowPositionPtr.reset();
	}
	__super::OnClose();
	DestroyWindow();
}

void InternalBrowser::OnNcDestroy()
{
	Detach();
	__super::OnNcDestroy();
}

LRESULT CALLBACK InternalBrowser::WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_APP+1) {
		auto thisPtr = (InternalBrowser*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		CString url;
		if (thisPtr->in->PopRequest(url)) {

			auto& webview = thisPtr->in->mWebView;
			if (webview) {
				HRESULT hr = webview->Navigate(url);
			}

		}
		return 0;
	}

	return ::DefWindowProc(hwnd, msg, wp, lp);
}



} // namespace webbrowser
} // namespace control
} // namespace soyokaze
