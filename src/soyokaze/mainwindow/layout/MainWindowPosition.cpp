#include "pch.h"
#include "MainWindowPosition.h"
#include "utility/AppProfile.h"
#include "app/AppName.h"
#include <utility> // for std::pair

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr int DEFAULT_HEIGHT = 480;

MainWindowPosition::MainWindowPosition() : WindowPosition()
{
}

MainWindowPosition::MainWindowPosition(LPCTSTR name) : WindowPosition(name)
{
}

MainWindowPosition::~MainWindowPosition()
{
}

// ウインドウ高さを取得する
int MainWindowPosition::GetHeight()
{
	return mPosition.rcNormalPosition.bottom - mPosition.rcNormalPosition.top;
}

// ウインドウ高さのみを更新する
void MainWindowPosition::SetHeight(int h)
{
	auto& wp = mPosition;
	wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + h;
}

// インスタンスが保持する位置情報にウインドウ位置・サイズを合わせる
bool MainWindowPosition::SyncPosition(HWND hwnd)
{
	spdlog::debug("MainWindowPosition::SyncPosition start");

	if (mIsLoaded == false) {
		// 情報がない場合は現状に合わせる
		GetWindowRect(hwnd, &mPosition.rcNormalPosition);
		mPosition.rcNormalPosition.bottom = mPosition.rcNormalPosition.top + DEFAULT_HEIGHT;

	}
	return SetWindowPlacement(hwnd, &mPosition) != FALSE;
}

