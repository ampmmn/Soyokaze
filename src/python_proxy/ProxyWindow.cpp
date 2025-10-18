#include "pch.h"
#include "ProxyWindow.h"
#include <thread>
#include <atomic>

struct ProxyWindow::PImpl
{
	HWND mHwnd{nullptr};
	std::atomic<bool> mIsAbort{false};
};

ProxyWindow::ProxyWindow() : in(new PImpl)
{
}

ProxyWindow::~ProxyWindow()
{
}

ProxyWindow* ProxyWindow::GetInstance()
{
	static ProxyWindow inst;
	return &inst;
}
	
int ProxyWindow::Initialize()
{
	// Pythonを実行するメインスレッドとなるスレッドを作成する
	// 呼び出し元は複数のスレッドになるため、内部のウインドウのメッセージ処理関数を経由することにより、
	// 単一スレッドで処理できるようにする
	std::thread th([&]() {

		// 内部のmessage処理用の不可視のウインドウ
		HWND hwnd = CreateWindowExW(0, L"STATIC", L"LauncherPythonProxy", 0, 0, 0, 1, 1, 
		                           nullptr, nullptr, GetModuleHandle(nullptr), NULL);
		if (IsWindow(hwnd) == FALSE) {
			return false;
		}

		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

		in->mHwnd = hwnd;
		in->mIsAbort = false;

		MSG msg;
		while(IsWindow(hwnd)) {
			GetMessage(&msg, nullptr, 0, 0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return true;
	});
	th.detach();
	
	return 0;
}

int ProxyWindow::Finalize()
{
	if (IsWindow(in->mHwnd) == FALSE) {
		return 0;
	}

	PostMessage(in->mHwnd, WM_CLOSE, 0, 0);
	PostQuitMessage(0);
	return 0;
}

void ProxyWindow::Abort()
{
	in->mIsAbort = true;
}

bool ProxyWindow::IsAbort()
{
	return in->mIsAbort.load();
}

bool ProxyWindow::RequestCallback(std::function<bool()> func)
{
	if (IsWindow(in->mHwnd) == FALSE) {
		return false;
	}

	bool result = false; 
	SendMessage(in->mHwnd, WM_APP + 1, (WPARAM)&result, (LPARAM)&func);

	return result;
}

LRESULT ProxyWindow::OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	auto thisPtr = (ProxyWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (thisPtr == nullptr || msg != WM_APP + 1) {
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	auto realCallback = (std::function<bool()>*)lp;
	auto resultPtr = (bool*)wp;
	*resultPtr = (*realCallback)();
	return 0;
}
