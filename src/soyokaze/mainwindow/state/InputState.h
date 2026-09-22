#pragma once

#include "mainwindow/state/LauncherWindowState.h"

namespace launcherapp { namespace mainwindow { namespace state {

/**
  メインウインドウが表示され、入力中の状態
*/
class InputState : public LauncherWindowState
{
public:
	explicit InputState(LauncherWindowStateContextIF* context);

	void OnActivate(bool isShowForce) override;
	void OnDeactivate() override;
	void OnExecuteRequested() override;
	void OnCancel() override;
	void OnContentCleared() override;
	void OnTextChanged() override;
	void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
	bool OnKeyInput(unsigned int keyCode) override;
	void OnCandidateSelectionChanged(int index) override;
	void OnCandidateClicked() override;
	void OnCandidateDoubleClicked() override;
};

}}}
