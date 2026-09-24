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
	  State遷移によってこのStateに入ったとき、ウインドウが表示中なら非表示にする
	*/
	void OnEnter() override;
	/** ウインドウの非表示要求を受けたとき、表示中であればウインドウを非表示にする */
	void OnDeactivate() override;
	/** ウインドウの表示要求を受けたとき、表示して入力内容に応じたStateへ遷移する */
	void OnActivate(bool isShowForce) override;
	/** 現在のコマンドの実行要求を受けたとき、実行処理をContextへ委譲する */
	void OnExecuteRequested() override;
	/** 非同期検索の完了通知を受けたとき、検索結果をContextへ反映する */
	void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
};

}}}
