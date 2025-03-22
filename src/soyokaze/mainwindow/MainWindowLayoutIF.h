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
	virtual void UpdateInputStatus(LauncherInput* status, bool isForceUpdate) = 0;
	// ウインドウ位置の復元
	virtual void RestoreWindowPosition(CWnd* wnd, bool isForceReset) = 0;
	// ウインドウがアクティブになるときのウインドウ位置を決める
	virtual bool RecalcWindowOnActivate(CWnd* wnd, CPoint& newPt) = 0;
	// リサイズ時のサイズ計算
	virtual void RecalcWindowSize(HWND hwnd, LauncherInput* status, UINT side, LPRECT rect) = 0;
	// リサイズ時の部品の再配置
	virtual void RecalcControls(HWND hwnd, LauncherInput* status) = 0;
};

}
}

