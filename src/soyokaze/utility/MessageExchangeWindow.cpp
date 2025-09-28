#include "pch.h"
#include "MessageExchangeWindow.h"

struct MessageExchangeWindow::PImpl
{
	HWND mHwnd{nullptr};
	std::function<LRESULT(HWND,UINT,WPARAM,LPARAM)> mCallback;
};


MessageExchangeWindow::MessageExchangeWindow() : in(new PImpl)
{
}

MessageExchangeWindow::~MessageExchangeWindow()
{
	Destroy();
}

LRESULT CALLBACK 
MessageExchangeWindow::OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	auto thisPtr = (MessageExchangeWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (thisPtr == nullptr) {
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return thisPtr->in->mCallback(hwnd, msg, wp, lp);
}

bool MessageExchangeWindow::Create(LPCTSTR caption, std::function<LRESULT(HWND,UINT,WPARAM,LPARAM)> callback)
{
	if (Create(caption) == false) {
		return false;
	}
	in->mCallback = callback;
	return true;
}

bool MessageExchangeWindow::Create(LPCTSTR caption)
{
	if (Exists()) {
		// 作成済
		return true;
	}

	// 内部のmessage処理用の不可視のウインド
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), caption, 0, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(nullptr), NULL);
	if (IsWindow(hwnd) == FALSE) {
		return false;
	}

	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	in->mHwnd = hwnd;

	return true;
}

void MessageExchangeWindow::Destroy()
{
	if (IsWindow(in->mHwnd)) {
		DestroyWindow(in->mHwnd);
	}
	in->mHwnd = nullptr;
}

void MessageExchangeWindow::SetCallback(std::function<LRESULT(HWND,UINT,WPARAM,LPARAM)> callback)
{
	in->mCallback = callback;
}

bool MessageExchangeWindow::Post(UINT msg, WPARAM wp, LPARAM lp)
{
	return PostMessage(in->mHwnd, msg, wp, lp) != FALSE;
}

bool MessageExchangeWindow::Exists()
{
	return IsWindow(in->mHwnd) != FALSE;
}

HWND MessageExchangeWindow::GetHwnd()
{
	return in->mHwnd;
}

