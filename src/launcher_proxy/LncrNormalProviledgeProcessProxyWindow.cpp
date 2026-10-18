#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atomic>
#include <exception>
#include <thread>
#include <spdlog/spdlog.h>
#include "LncrNormalProviledgeProcessProxyWindow.h"
#include "SharedHwnd.h"

namespace {

constexpr LPCTSTR WINDOW_TITLE = _T("LncrNormalProviledgeProcessProxy");
constexpr LPCTSTR NAME_NORMALPRIV_SERVER = _T("LauncherAppNormalPriviledgeServer");

}

struct LncrNormalProviledgeProcessProxyWindow::PImpl
{
	/**
	  専用スレッドでウィンドウを作成してメッセージを処理する
	  @param[in] hInst インスタンスハンドル
	*/
	void Run(HINSTANCE hInst);
	/**
	  ビジー状態の問い合わせに応答するウィンドウプロシージャ
	  @param hwnd ウィンドウハンドル
	  @param msg メッセージ
	  @param wp WPARAM
	  @param lp LPARAM
	  @return メッセージ処理結果
	*/
	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	HANDLE mStartedEvent{nullptr};
	HANDLE mStopEvent{nullptr};
	bool mCreateSucceeded{false};
	std::atomic<bool> mIsBusy{false};
	std::thread mThread;
};

void LncrNormalProviledgeProcessProxyWindow::PImpl::Run(HINSTANCE hInst)
{
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), WINDOW_TITLE, 0,
	                           0, 0, 0, 0, NULL, NULL, hInst, NULL);
	if (hwnd != nullptr) {
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
		SetLastError(0);
		if (SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc) == 0 && GetLastError() != 0) {
			DestroyWindow(hwnd);
			hwnd = nullptr;
		}
	}

	if (hwnd == nullptr) {
		mCreateSucceeded = false;
		SetEvent(mStartedEvent);
		return;
	}

	SharedHwnd serverHwnd(hwnd, NAME_NORMALPRIV_SERVER);
	mCreateSucceeded = true;
	SetEvent(mStartedEvent);
	SharedHwnd parentHwnd;
	bool shouldExit = false;
	while (shouldExit == false && IsWindow(parentHwnd.GetHwnd()) != FALSE) {
		DWORD waitResult = MsgWaitForMultipleObjects(1, &mStopEvent, FALSE, 50, QS_ALLINPUT);
		if (waitResult == WAIT_OBJECT_0) {
			break;
		}
		if (waitResult == WAIT_FAILED) {
			break;
		}
		if (waitResult == WAIT_OBJECT_0 + 1) {
			MSG msg;
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					shouldExit = true;
					break;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	if (IsWindow(hwnd) != FALSE) {
		DestroyWindow(hwnd);
	}
	serverHwnd.SetHwnd(nullptr);
}

LRESULT CALLBACK LncrNormalProviledgeProcessProxyWindow::PImpl::OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	auto thisPtr = (PImpl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (msg == WM_APP + 1 && thisPtr != nullptr) {
		return thisPtr->mIsBusy.load() ? 1 : 0;
	}
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

LncrNormalProviledgeProcessProxyWindow::LncrNormalProviledgeProcessProxyWindow() : in(new PImpl)
{
}

LncrNormalProviledgeProcessProxyWindow::~LncrNormalProviledgeProcessProxyWindow()
{
	if (in->mStopEvent != nullptr) {
		SetEvent(in->mStopEvent);
	}
	if (in->mThread.joinable()) {
		in->mThread.join();
	}
	if (in->mStartedEvent != nullptr) {
		CloseHandle(in->mStartedEvent);
	}
	if (in->mStopEvent != nullptr) {
		CloseHandle(in->mStopEvent);
	}
}

bool LncrNormalProviledgeProcessProxyWindow::Create(HINSTANCE hInst)
{
	in->mStartedEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	in->mStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (in->mStartedEvent == nullptr || in->mStopEvent == nullptr) {
		spdlog::error("Failed to create proxy window thread event.");
		return false;
	}

	try {
		in->mThread = std::thread([this, hInst]() {
			in->Run(hInst);
		});
	}
	catch (const std::exception& e) {
		spdlog::error("Failed to create proxy window thread: {}", e.what());
		return false;
	}

	if (WaitForSingleObject(in->mStartedEvent, INFINITE) != WAIT_OBJECT_0) {
		return false;
	}
	return in->mCreateSucceeded;
}

void LncrNormalProviledgeProcessProxyWindow::SetBusy(bool isBusy)
{
	in->mIsBusy.store(isBusy);
}
