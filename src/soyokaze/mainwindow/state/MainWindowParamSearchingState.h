#pragma once

#include "mainwindow/state/LauncherWindowStateBase.h"

namespace launcherapp { namespace mainwindow { namespace state {

/**
  コマンドパラメータの追加候補を選択している状態
*/
class ParamSearchingState : public LauncherWindowStateBase
{
public:
	explicit ParamSearchingState(LauncherWindowStateContextIF* context);

	void OnEnter() override;
	void OnExit() override;
	void OnDeactivate() override;
	void OnExecuteRequested() override;
	void OnCancel() override;
	void OnTextChanged() override;
	bool OnKeyInput(unsigned int keyCode) override;
	void OnSelectionChanged() override;
	void OnWindowGeometryChanged() override;
	void OnExtraCandidateSelectionChanged() override;
	void OnExtraCandidateClicked() override;

private:
	int mPendingSelectionChangeNotifications{0};
};

}}}
