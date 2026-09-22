#pragma once

#include "mainwindow/state/LauncherWindowState.h"

namespace launcherapp { namespace mainwindow { namespace state {

/**
  メインウインドウが表示され、入力がない状態
*/
class ShownState : public LauncherWindowState
{
public:
	explicit ShownState(LauncherWindowStateContextIF* context);

	void OnActivate(bool isShowForce) override;
	void OnDeactivate() override;
	void OnExecuteRequested() override;
	void OnCancel() override;
	void OnTextChanged() override;
	void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
	bool OnKeyInput(unsigned int keyCode) override;
};

}}}
