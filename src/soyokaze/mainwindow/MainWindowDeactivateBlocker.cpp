#include "pch.h"
#include "MainWindowDeactivateBlocker.h"
#include "mainwindow/controller/MainWindowController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

MainWindowDeactivateBlocker::MainWindowDeactivateBlocker()
{
	// フォーカスを失ったときに隠れるのを阻害する
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	mainWnd->BlockDeactivateOnUnfocus(true);

}

MainWindowDeactivateBlocker::~MainWindowDeactivateBlocker()
{
	// 状態をもとに戻す
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	mainWnd->BlockDeactivateOnUnfocus(false);
}
