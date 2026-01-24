#include "pch.h"
#include "ManualWindow.h"
#include "app/AppName.h"
#include "icon/IconLoader.h"
#include "control/webbrowser/InternalBrowser.h"
#include "utility/VersionInfo.h"
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using InternalBrowser = soyokaze::control::webbrowser::InternalBrowser;

static bool sIsWndClassRegistered = false;

static bool RegigsterWindowClass()
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = InternalBrowser::WindowProc;
	wc.hInstance     = GetModuleHandle(nullptr);
	wc.hIcon         = IconLoader::Get()->LoadHelpIcon();
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"LauncherManualWindow";

	if (RegisterClassEx(&wc) == FALSE) {
		spdlog::error("Failed to regsiter LauncherManualWindow class.");
		return false;
	}
	return true;
}



struct ManualWindow::PImpl 
{
	void CreateManualWindow();
	std::unique_ptr<InternalBrowser> mBrowserWindow;
};

void ManualWindow::PImpl::CreateManualWindow()
{
	// ヘルプ画面をメインスレッドで作成すると、メインスレッド側でモーダルダイアログを表示しているときに
	// ヘルプ画面を操作できなくなってしまうため、専用のスレッドでヘルプ画面を作る
	std::thread th([&]() {

		// 初回呼び出し時にウインドウクラスを登録する
		if (sIsWndClassRegistered == false) {
			RegigsterWindowClass();
			sIsWndClassRegistered = true;
		}

		mBrowserWindow->SetHostWindowClass(L"LauncherManualWindow");
		mBrowserWindow->EnableSaveWindowPosition(_T("ManualWindow"));

		CRect rect(0, 0, 800, 600);
		int style = WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
		mBrowserWindow->Create(nullptr, style, rect, 0);

		CString version;
		VersionInfo::GetVersionInfo(version);

		CString caption;
		caption.Format(_T("%s %s マニュアル"), APPNAME, (LPCTSTR)version);

		mBrowserWindow->SetWindowText(caption);

		HWND h = mBrowserWindow->GetSafeHwnd();
		while(IsWindow(h)) {
			MSG msg;
			::GetMessage(&msg, nullptr, 0, 0);
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	});
	th.detach();
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


ManualWindow::ManualWindow() : in(std::make_unique<PImpl>())
{
}

ManualWindow::~ManualWindow()
{
}

ManualWindow* ManualWindow::GetInstance()
{
	static ManualWindow inst;
	return &inst;
}

void ManualWindow::Open(const CString& url)
{
	// ウインドウがなければ作成
	auto& wndPtr = in->mBrowserWindow;
	if (wndPtr.get() == nullptr || wndPtr->GetSafeHwnd() == NULL) {
		wndPtr.reset(new InternalBrowser());
		in->CreateManualWindow();
	}
	// ウインドウが非表示なら表示
	HWND h = in->mBrowserWindow->GetSafeHwnd();
	if (IsWindow(h) && ::IsWindowVisible(h) == FALSE) {
		::ShowWindow(h, SW_SHOW);
	}

	in->mBrowserWindow->Open(url);
}
