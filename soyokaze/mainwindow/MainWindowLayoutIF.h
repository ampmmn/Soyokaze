#pragma once

#include "mainwindow/LauncherInputStatusIF.h"

namespace launcherapp {
namespace mainwindow {

// メインウインドウのウインドウ位置・サイズ、コントロールの配置(リサイズ)処理などを実装するためのインタフェース
class MainWindowLayoutIF
{
public:
	virtual ~MainWindowLayoutIF() {}

	// 入力が更新された
	virtual void UpdateInputStatus(LauncherInputStatus* status) = 0;
	// ウインドウ位置の復元
	virtual void RestoreWindowPosition(CWnd* wnd, bool isForceReset) = 0;
	virtual void OnShowWindow(CWnd* wnd, BOOL bShow, UINT nStatus) = 0;
	virtual void RecalcWindowSize(HWND hwnd, LauncherInputStatus* status, UINT side, LPRECT rect) = 0;
	virtual void RecalcControls(HWND hwnd, LauncherInputStatus* status) = 0;
};

}
}

