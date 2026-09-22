#pragma once

#include "mainwindow/state/LauncherWindowStateBase.h"

namespace launcherapp { namespace mainwindow { namespace state {

/**
  メインウインドウが表示され、入力がない状態
*/
class IdleState : public LauncherWindowStateBase
{
public:
	explicit IdleState(LauncherWindowStateContextIF* context);

	/** 表示要求に応じて表示、再アクティブ化、トグル非表示を行う */
	void OnActivate(bool isShowForce) override;
	/** 非表示Stateへ遷移する */
	void OnDeactivate() override;
	/** 現在の候補を実行し、ウインドウが閉じた場合は非表示Stateへ遷移する */
	void OnExecuteRequested() override;
	/** 入力内容をクリアするか、ウインドウを非表示にする */
	void OnCancel() override;
	/** 入力開始を検知したら検索中Stateへ遷移する */
	void OnTextChanged() override;
	/** 検索結果を反映し、必要に応じてStateを更新する */
	void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
	/** 待機中に処理するEnterキーを受け付ける */
	bool OnKeyInput(unsigned int keyCode) override;
};

}}}
