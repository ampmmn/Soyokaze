#include "pch.h"
#include "MainWindowController.h"
#include "mainwindow/controller/LauncherMainWindowController.h"

namespace launcherapp { namespace mainwindow { namespace controller {

MainWindowController::MainWindowController() : 
	mInst(new LauncherMainWindowController())
{
}

MainWindowController::~MainWindowController()
{
	delete mInst;
}

MainWindowControllerIF* MainWindowController::GetInstance()
{
	static MainWindowController instance;
	return instance.mInst;
}

// テスト用
MainWindowControllerIF* MainWindowController::SetInstance(MainWindowControllerIF* inst)
{
	auto oldInstance = mInst; 
	mInst = inst;
	return oldInstance;
}

}}}
