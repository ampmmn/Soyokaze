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

	/** ウインドウの非表示要求を受けたとき、非表示Stateへ遷移する */
	void OnDeactivate() override;
	/** 現在の候補の実行要求を受けたとき、実行し、ウインドウが閉じた場合は非表示Stateへ遷移する */
	void OnExecuteRequested() override;
	/** キャンセル操作を受けたとき、入力内容をクリアするかウインドウを非表示にする */
	void OnCancel() override;
	/** 入力欄の文字変更通知を受けたとき、入力開始なら検索中Stateへ遷移する */
	void OnTextChanged() override;
	/** 非同期検索の完了通知を受けたとき、検索結果を反映して必要に応じてStateを更新する */
	void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
	/** 入力欄でキー入力を受けたとき、待機中に処理するEnterキーを処理する */
	bool OnKeyInput(unsigned int keyCode) override;
};

}}}
