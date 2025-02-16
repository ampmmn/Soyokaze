#pragma once

#include "mainwindow/LauncherMainWindowIF.h"
#include "mainwindow/MainWindowLayoutIF.h"

namespace launcherapp {
namespace mainwindow {

// メインウインドウのリサイズ時にウインドウ上の部品の位置とサイズを再計算し配置する
class MainWindowLayout : public MainWindowLayoutIF
{
public:
	MainWindowLayout(LauncherMainWindowIF* mainWnd);
	~MainWindowLayout() override;

	// 入力が更新された
	void UpdateInputStatus(LauncherInput* status, bool isForceUpdate) override;
	// ウインドウ位置の復元
	void RestoreWindowPosition(CWnd* wnd, bool isForceReset) override;
	// 表示/非表示になるときの処理
	void OnShowWindow(CWnd* wnd, BOOL bShow, UINT nStatus) override;
	// リサイズ中のメインウインドウのサイズ制限を決定する
	void RecalcWindowSize(HWND hwnd, LauncherInput* status, UINT side, LPRECT rect) override;
	// リサイズ時のコントロール位置・サイズの再計算
	void RecalcControls(HWND hwnd, LauncherInput* status) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
