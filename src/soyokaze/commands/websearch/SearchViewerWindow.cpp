#include "pch.h"
#include "SearchViewerWindow.h"
#include "control/WindowPosition.h"
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include <wrl.h>
#include <algorithm>
#include <cstdint>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::WRL;

namespace launcherapp {
namespace commands {
namespace websearch {

namespace {

constexpr UINT WM_NAVIGATE_PENDING = WM_APP + 42;
constexpr UINT ADDRESS_BAR_ID = 1;

}

struct SearchViewerWindow::PImpl
{
	CString mCommandName;
	CString mTitle;
	DWORD mWindowThreadId{0};
	HWND mWindow{nullptr};
	HWND mAddressBar{nullptr};
	HFONT mAddressBarFont{nullptr};
	int mAddressBarHeight{28};
	ComPtr<ICoreWebView2Controller> mWebViewController;
	ComPtr<ICoreWebView2> mWebView;
	std::unique_ptr<WindowPosition> mWindowPosition;
	CString mPendingUrl;
	uint64_t mGeneration{0};
};

IMPLEMENT_DYNAMIC(SearchViewerWindow, CWnd)

BEGIN_MESSAGE_MAP(SearchViewerWindow, CWnd)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_QUERYENDSESSION()
	ON_WM_ACTIVATE()
	ON_MESSAGE(WM_NAVIGATE_PENDING, OnNavigatePending)
END_MESSAGE_MAP()

SearchViewerWindow::SearchViewerWindow(const CString& commandName) : in(std::make_shared<PImpl>())
{
	in->mCommandName = commandName;
}

SearchViewerWindow::~SearchViewerWindow()
{
	Close();
}

bool SearchViewerWindow::Open(const CString& title, const CString& url)
{
	if (in->mWindowThreadId != 0 && in->mWindowThreadId != GetCurrentThreadId()) {
		ASSERT(FALSE);
		return false;
	}
	if (IsAllowedUrl(url) == false) {
		return false;
	}

	in->mTitle = title;
	in->mPendingUrl = url;
	if (GetSafeHwnd() == nullptr) {
		in->mWindowThreadId = GetCurrentThreadId();
		LPCTSTR windowClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(nullptr, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), nullptr);
		if (CreateEx(0, windowClass, title, WS_OVERLAPPEDWINDOW, CRect(100, 100, 1100, 850), nullptr, 0) == FALSE) {
			in->mWindowThreadId = 0;
			return false;
		}

		in->mWindow = GetSafeHwnd();
		in->mWindowPosition = std::make_unique<WindowPosition>(CreateWindowPlacementName(in->mCommandName));
		in->mWindowPosition->Restore(in->mWindow);

		if (CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY | WS_TABSTOP,
			0, 0, 0, 0, in->mWindow, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ADDRESS_BAR_ID)), AfxGetInstanceHandle(), nullptr) == nullptr) {
			DestroyWindow();
			in->mWindowThreadId = 0;
			return false;
		}
		in->mAddressBar = ::GetDlgItem(in->mWindow, ADDRESS_BAR_ID);
		ConfigureAddressBarFont();
		RECT clientRect{};
		GetClientRect(&clientRect);
		Layout(clientRect.right, clientRect.bottom);

		const int showCommand = GetRestoreShowCommand(in->mWindowPosition->GetPosition());
		ShowWindow(showCommand);
		SetForegroundWindow();
		UpdateWindow();
		InitializeWebView();
	}
	else {
		SetTitle(title);
		if (in->mWebView) {
			::PostMessage(in->mWindow, WM_NAVIGATE_PENDING, 0, 0);
		}
		ShowWindow(IsIconic() ? SW_RESTORE : SW_SHOW);
		SetForegroundWindow();
	}
	return true;
}

void SearchViewerWindow::SetTitle(const CString& title)
{
	if (in->mWindowThreadId != 0 && in->mWindowThreadId != GetCurrentThreadId()) {
		ASSERT(FALSE);
		return;
	}
	in->mTitle = title;
	if (GetSafeHwnd() && ::IsWindow(GetSafeHwnd())) {
		SetWindowText(title);
	}
}

void SearchViewerWindow::Close()
{
	if (in->mWindowThreadId != 0 && in->mWindowThreadId != GetCurrentThreadId()) {
		ASSERT(FALSE);
		return;
	}
	if (GetSafeHwnd()) {
		SaveWindowPosition();
		DestroyWindow();
	}
}

bool SearchViewerWindow::IsAllowedUrl(const CString& url)
{
	if (url.GetLength() >= 7 && url.Left(7).CompareNoCase(_T("http://")) == 0) {
		return true;
	}
	return url.GetLength() >= 8 && url.Left(8).CompareNoCase(_T("https://")) == 0;
}

int SearchViewerWindow::GetRestoreShowCommand(const WINDOWPLACEMENT& placement)
{
	return placement.showCmd == SW_SHOWMAXIMIZED ? SW_SHOWMAXIMIZED : SW_SHOW;
}

CString SearchViewerWindow::CreateWindowPlacementName(const CString& commandName)
{
	static constexpr wchar_t HEX[] = L"0123456789ABCDEF";
	std::wstring name = L"SearchViewer-";
	for (int i = 0; i < commandName.GetLength(); ++i) {
		const auto character = static_cast<unsigned short>(commandName[i]);
		for (int shift = 12; shift >= 0; shift -= 4) {
			name.push_back(HEX[(character >> shift) & 0x0f]);
		}
	}
	return CString(name.c_str());
}

void SearchViewerWindow::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	Layout(cx, cy);
}

void SearchViewerWindow::Layout(int width, int height)
{
	if (in->mAddressBar) {
		::MoveWindow(in->mAddressBar, 0, 0, width, in->mAddressBarHeight, TRUE);
	}
	if (in->mWebViewController) {
		RECT bounds{ 0, in->mAddressBarHeight, width, height };
		in->mWebViewController->put_Bounds(bounds);
	}
}

void SearchViewerWindow::OnClose()
{
	SaveWindowPosition();
	DestroyWindow();
}

void SearchViewerWindow::OnDestroy()
{
	++in->mGeneration;
	if (in->mWebViewController) {
		in->mWebViewController->Close();
	}
	in->mWebView.Reset();
	in->mWebViewController.Reset();
	if (in->mAddressBarFont) {
		DeleteObject(in->mAddressBarFont);
		in->mAddressBarFont = nullptr;
	}
	in->mWindow = nullptr;
	in->mAddressBar = nullptr;
	in->mWindowThreadId = 0;
	__super::OnDestroy();
}

BOOL SearchViewerWindow::OnQueryEndSession()
{
	if (in->mWindowPosition) {
		in->mWindowPosition->Update(GetSafeHwnd());
		in->mWindowPosition->Save(); // セッション終了時にも位置を永続化する
	}
	return TRUE;
}

void SearchViewerWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	__super::OnActivate(nState, pWndOther, bMinimized);
	if (nState != WA_INACTIVE && in->mWebViewController) {
		in->mWebViewController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
	}
}

void SearchViewerWindow::ConfigureAddressBarFont()
{
	if (in->mAddressBar == nullptr) {
		return;
	}

	NONCLIENTMETRICSW metrics{};
	metrics.cbSize = sizeof(metrics);
	if (::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, static_cast<UINT>(sizeof(metrics)), &metrics, 0) != FALSE) {
		in->mAddressBarFont = CreateFontIndirectW(&metrics.lfCaptionFont);
	}
	if (in->mAddressBarFont) { // タイトルバーのフォントをアドレス欄に適用する
		::SendMessage(in->mAddressBar, WM_SETFONT, reinterpret_cast<WPARAM>(in->mAddressBarFont), TRUE);
	}

	HFONT font = reinterpret_cast<HFONT>(::SendMessage(in->mAddressBar, WM_GETFONT, 0, 0));
	if (font == nullptr) {
		font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
	}
	HDC deviceContext = ::GetDC(in->mAddressBar);
	if (deviceContext == nullptr) {
		return;
	}
	HGDIOBJ previousFont = SelectObject(deviceContext, font);
	TEXTMETRICW textMetrics{};
	if (previousFont != nullptr && previousFont != HGDI_ERROR && GetTextMetricsW(deviceContext, &textMetrics) != FALSE) {
		in->mAddressBarHeight = (std::max)(28, static_cast<int>(textMetrics.tmHeight) + 8);
	}
	if (previousFont != nullptr && previousFont != HGDI_ERROR) {
		SelectObject(deviceContext, previousFont);
	}
	::ReleaseDC(in->mAddressBar, deviceContext);
}

void SearchViewerWindow::SaveWindowPosition()
{
	if (in->mWindowPosition) {
		in->mWindowPosition->Update(GetSafeHwnd());
		in->mWindowPosition.reset();
	}
}

void SearchViewerWindow::InitializeWebView()
{
	auto state = in;
	auto weakState = std::weak_ptr<PImpl>(state);
	const auto generation = ++state->mGeneration;
	auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, options.Get(),
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[weakState, generation](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
				auto state = weakState.lock();
				if (state == nullptr || state->mWindow == nullptr || state->mGeneration != generation) {
					return S_OK;
				}
				if (FAILED(result) || environment == nullptr) {
					::MessageBox(state->mWindow, _T("WebView2を初期化できません。"), _T("検索ウインドウ"), MB_OK | MB_ICONERROR);
					::PostMessage(state->mWindow, WM_CLOSE, 0, 0);
					return FAILED(result) ? result : E_FAIL;
				}

				environment->CreateCoreWebView2Controller(state->mWindow,
					Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
						[weakState, generation](HRESULT controllerResult, ICoreWebView2Controller* controller) -> HRESULT {
							auto state = weakState.lock();
							if (state == nullptr || state->mWindow == nullptr || state->mGeneration != generation) {
								return S_OK;
							}
							if (FAILED(controllerResult) || controller == nullptr) {
								::MessageBox(state->mWindow, _T("WebView2の表示領域を作成できません。"), _T("検索ウインドウ"), MB_OK | MB_ICONERROR);
								::PostMessage(state->mWindow, WM_CLOSE, 0, 0);
								return FAILED(controllerResult) ? controllerResult : E_FAIL;
							}

							state->mWebViewController = controller;
							if (FAILED(state->mWebViewController->get_CoreWebView2(&state->mWebView)) || !state->mWebView) {
								state->mWebViewController->Close();
								state->mWebViewController.Reset();
								return E_FAIL;
							}

							RECT bounds{};
							::GetClientRect(state->mWindow, &bounds);
							bounds.top = state->mAddressBarHeight;
							state->mWebViewController->put_Bounds(bounds);

							ComPtr<ICoreWebView2Settings> settings;
							state->mWebView->get_Settings(&settings);
							if (settings) {
								settings->put_AreDevToolsEnabled(FALSE);
								settings->put_AreHostObjectsAllowed(FALSE);
								settings->put_IsWebMessageEnabled(FALSE);
								settings->put_AreDefaultScriptDialogsEnabled(FALSE);
								ComPtr<ICoreWebView2Settings3> settings3;
								if (SUCCEEDED(settings.As(&settings3)) && settings3) {
									settings3->put_AreBrowserAcceleratorKeysEnabled(TRUE);
								}
							}

							state->mWebView->add_NavigationStarting(
								Callback<ICoreWebView2NavigationStartingEventHandler>(
									[weakState, generation](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
										LPWSTR uri = nullptr;
										if (SUCCEEDED(args->get_Uri(&uri)) && uri != nullptr) {
											CString url(uri);
											if (SearchViewerWindow::IsAllowedUrl(url) == false) {
												args->put_Cancel(TRUE);
												}
											else if (auto state = weakState.lock(); state && state->mGeneration == generation) {
												::SetWindowText(state->mAddressBar, url);
												}
											CoTaskMemFree(uri);
										}
										return S_OK;
									}).Get(), nullptr);

							state->mWebView->add_NewWindowRequested(
								Callback<ICoreWebView2NewWindowRequestedEventHandler>(
									[weakState, generation](ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
										args->put_Handled(TRUE);
										LPWSTR uri = nullptr;
										if (SUCCEEDED(args->get_Uri(&uri)) && uri != nullptr) {
											CString url(uri);
											if (SearchViewerWindow::IsAllowedUrl(url)) {
												if (auto state = weakState.lock(); state && state->mGeneration == generation) {
													state->mWebView->Navigate(url);
												}
											}
											CoTaskMemFree(uri);
										}
										return S_OK;
									}).Get(), nullptr);

							::PostMessage(state->mWindow, WM_NAVIGATE_PENDING, 0, 0);
							state->mWebViewController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
							return S_OK;
						}).Get());
				return S_OK;
			}).Get());
}

LRESULT SearchViewerWindow::OnNavigatePending(WPARAM, LPARAM)
{
	if (in->mWebView && in->mPendingUrl.IsEmpty() == FALSE) {
		in->mWebView->Navigate(CStringW(in->mPendingUrl));
		in->mPendingUrl.Empty();
	}
	return 0;
}

}
}
}
