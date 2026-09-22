#pragma once

namespace launcherapp { namespace mainwindow { namespace state {
class LauncherWindowStateContextIF;
}}}

namespace launcherapp { namespace commands { namespace core {
class CommandQueryResult;
}}}
namespace launcherapp { namespace mainwindow { namespace state {

/**
  ランチャーウインドウの状態を表す基底クラス
*/
class LauncherWindowState
{
public:
	explicit LauncherWindowState(LauncherWindowStateContextIF* context);
	virtual ~LauncherWindowState();

	virtual void OnEnter();
	virtual void OnExit();
	virtual void OnActivate(bool isShowForce);
	virtual void OnDeactivate();
	virtual void OnExecuteRequested();
	virtual void OnCancel();
	virtual void OnContentCleared();
	virtual void OnTextChanged();
	virtual void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result);
	virtual bool OnKeyInput(unsigned int keyCode);
	virtual void OnCandidateSelectionChanged(int index);
	virtual void OnCandidateClicked();
	virtual void OnCandidateDoubleClicked();

protected:
	LauncherWindowStateContextIF* GetContext() const;

private:
	LauncherWindowStateContextIF* mContext;
};

}}}
