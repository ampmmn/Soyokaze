#include "pch.h"
#include "InternalBrowser.h"
#include "control/WindowPosition.h"
#include "externaltool/webbrowser/ConfiguredBrowserEnvironment.h"
#include <winrt/base.h>
#include <wrl.h>
#include <wil/com.h>
#include <mutex>
#include <deque>
#include <atomic>
#include <cstdint>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::WRL;
using ConfiguredBrowserEnvironment = launcherapp::externaltool::webbrowser::ConfiguredBrowserEnvironment;

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


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class NavigationStartingHandler : 
	public RuntimeClass<RuntimeClassFlags<ClassicCom>, ICoreWebView2NavigationStartingEventHandler>
{
public:
    NavigationStartingHandler() {}

    HRESULT STDMETHODCALLTYPE Invoke(
        ICoreWebView2*,
        ICoreWebView2NavigationStartingEventArgs* args) override
    {
        wil::unique_cotaskmem_string uri;
        args->get_Uri(&uri);

        std::wstring url(uri.get());

				// 外部サイトは自アプリで開かず、ブラウザで開く
        if (url.find(L"http", 0) == 0) {
            args->put_Cancel(TRUE);

						// アプリ設定の 外部ツール > Webブラウザ の設定でURLを開く
						auto brwsEnv = ConfiguredBrowserEnvironment::GetInstance();
						brwsEnv->OpenURL(url.c_str());
        }
        return S_OK;
    }
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



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
		mRequest.pop_back();
		return true;
	}
	void CloseWebview() {
		mIsClosing = true;
		++mGeneration;
		if (mWebViewCtrl) {
			mWebViewCtrl->Close();
		}
		mWebView.reset();
		mWebViewCtrl.reset();
	}

	wil::com_ptr<ICoreWebView2Controller> mWebViewCtrl;
	wil::com_ptr<ICoreWebView2> mWebView;

	CStringW mHostWindowClassName;
	HWND mSelfWindow{nullptr};

	std::mutex mMutex;
	std::deque<CString> mRequest;
	std::unique_ptr<WindowPosition> mWindowPositionPtr;
	std::atomic<std::uint64_t> mGeneration{0};
	std::atomic_bool mIsClosing{false};
};


InternalBrowser::InternalBrowser() : in(std::make_shared<PImpl>())
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

	CStringW clsName(in->mHostWindowClassName);
	if (clsName.IsEmpty()) {
		clsName = _T("LauncherInernalBrowser");
	}

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
	ON_WM_QUERYENDSESSION()
END_MESSAGE_MAP()

void InternalBrowser::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();
//	InitializeWebview();
}

void InternalBrowser::InitializeWebview()
{
	auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
	auto state = in;
	const auto generation = ++state->mGeneration;
	state->mIsClosing = false;
	const auto weakState = std::weak_ptr<PImpl>(state);
	const HWND hwnd = state->mSelfWindow;

	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, options.Get(),
			Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
				[weakState, generation, hwnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
					auto state = weakState.lock();
					if (!state || state->mIsClosing || state->mGeneration != generation) {
						return S_OK;
					}

				if (FAILED(result)) {
					return result;
				}

					env->CreateCoreWebView2Controller(hwnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
							[weakState, generation, hwnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
						auto state = weakState.lock();
						if (!state || state->mIsClosing || state->mGeneration != generation) {
							return S_OK;
						}
					if (FAILED(result)) {
						return result;
					}
					if (controller == nullptr) {
						return E_UNEXPECTED;
					}

					state->mWebViewCtrl = controller;
					if (FAILED(state->mWebViewCtrl->get_CoreWebView2(&state->mWebView)) || !state->mWebView) {
						state->mWebViewCtrl->Close();
						state->mWebViewCtrl.reset();
						return E_FAIL;
					}

					RECT bounds;
					::GetClientRect(hwnd, &bounds);
					state->mWebViewCtrl->put_Bounds(bounds);
					wil::com_ptr<ICoreWebView2Settings> settings;
					state->mWebView->get_Settings(&settings);

					// FIXME: 現在はヘルプウインドウの用途に限定しているため、下記の設定としているが、
					// 多用途でこのクラスを使いまわす場合は、機能を適宜整理すること

					// 不要な機能を無効化する
					settings->put_AreDevToolsEnabled(FALSE);
					settings->put_AreHostObjectsAllowed(FALSE);
					settings->put_IsWebMessageEnabled(FALSE);
					settings->put_AreDefaultScriptDialogsEnabled(FALSE);

					// 表示するURLを監視する(外部接続を許可しない)
					EventRegistrationToken token;
					auto handler = Microsoft::WRL::Make<NavigationStartingHandler>();
					state->mWebView->add_NavigationStarting(handler.Get(), &token);

					// URL表示のためのキュー処理
					if (IsWindow(hwnd)) {
						::PostMessage(hwnd, WM_APP+1, 0, 0);
					}
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
	in->CloseWebview();
	if (in->mWindowPositionPtr.get()) {
		in->mWindowPositionPtr->Update(GetSafeHwnd());
		in->mWindowPositionPtr.reset();
	}
	__super::OnClose();
	DestroyWindow();
}

void InternalBrowser::OnNcDestroy()
{
	in->mSelfWindow = nullptr;
	Detach();
	__super::OnNcDestroy();
}

BOOL InternalBrowser::OnQueryEndSession()
{
	PostMessage(WM_CLOSE,0, 0);
	return TRUE;
}

LRESULT CALLBACK InternalBrowser::WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_APP+1) {
		auto thisPtr = (InternalBrowser*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (thisPtr == nullptr) {
			return 0;
		}

		CString url;
		if (thisPtr->in->PopRequest(url)) {

			auto& webview = thisPtr->in->mWebView;
			if (webview) {
				webview->Navigate(url);
				if (thisPtr->in->mWebViewCtrl) {
					thisPtr->in->mWebViewCtrl->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
				}
			}

		}
		return 0;
	}
	else if (msg == WM_ACTIVATE) {
		if (wp != WA_INACTIVE) {
			// ウインドウがアクティブになったらwebview側にフォーカスを設定する
			auto thisPtr = (InternalBrowser*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (thisPtr) {
				if (thisPtr->in->mWebViewCtrl) {
					thisPtr->in->mWebViewCtrl->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
				}
			}
			return 0;
		}
	}

	return ::DefWindowProc(hwnd, msg, wp, lp);
}



} // namespace webbrowser
} // namespace control
} // namespace soyokaze
