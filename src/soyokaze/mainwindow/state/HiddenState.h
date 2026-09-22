#pragma once

#include "mainwindow/state/LauncherWindowState.h"

namespace launcherapp { namespace mainwindow { namespace state {

/**
  メインウインドウが非表示の状態
*/
class HiddenState : public LauncherWindowState
{
public:
	explicit HiddenState(LauncherWindowStateContextIF* context);

	/**
	  非表示Stateへの遷移時にウインドウを非表示にする
	*/
	void OnEnter() override;
	void OnActivate(bool isShowForce) override;
	void OnExecuteRequested() override;
	void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
};

}}}
