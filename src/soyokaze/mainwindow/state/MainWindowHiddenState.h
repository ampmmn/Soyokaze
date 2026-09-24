#pragma once

#include "mainwindow/state/LauncherWindowStateBase.h"

namespace launcherapp { namespace mainwindow { namespace state {

/**
  メインウインドウが非表示の状態
*/
class HiddenState : public LauncherWindowStateBase
{
public:
	explicit HiddenState(LauncherWindowStateContextIF* context);

	/**
	  非表示Stateへの遷移時にウインドウを非表示にする
	*/
	void OnEnter() override;
	/** 非表示要求を受けたとき、表示中であればウインドウを非表示にする */
	void OnDeactivate() override;
	/** 表示要求を受けたらウインドウを表示し、入力内容に応じてStateを選択する */
	void OnActivate(bool isShowForce) override;
	/** 非表示中の実行要求をContextへ委譲する */
	void OnExecuteRequested() override;
	/** 非表示中に完了した検索結果をContextへ反映する */
	void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
};

}}}
