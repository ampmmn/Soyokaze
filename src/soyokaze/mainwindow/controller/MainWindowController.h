#pragma once

#include "mainwindow/controller/MainWindowControllerIF.h"

namespace launcherapp { namespace mainwindow { namespace controller {

class MainWindowController
{
private:
	MainWindowController();
	~MainWindowController();

public:
	static MainWindowControllerIF* GetInstance();
	// テスト用
	MainWindowControllerIF* SetInstance(MainWindowControllerIF* inst);

private:
	MainWindowControllerIF* mInst;
};


}}}
