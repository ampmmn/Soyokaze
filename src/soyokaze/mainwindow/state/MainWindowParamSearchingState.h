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

	/** State遷移によってこのStateに入ったとき、追加候補を更新する */
	void OnEnter() override;
	/** State遷移によってこのStateを離れるとき、追加候補欄を隠す */
	void OnExit() override;
	/** ウインドウが非アクティブになったとき、追加候補欄を隠す */
	void OnDeactivate() override;
	/** ウインドウの非表示要求を受けたとき、非表示Stateへ遷移する */
	void OnHideRequested() override;
	/** 現在の追加候補の実行要求を受けたとき、候補を確定して検索中Stateへ遷移する */
	void OnExecuteRequested() override;
	/** キャンセル操作を受けたとき、通常の検索中Stateへ戻る */
	void OnCancel() override;
	/** 入力欄の文字変更通知を受けたとき、入力状態に応じてStateまたは追加候補を更新する */
	void OnTextChanged() override;
	/** 入力欄でキー入力を受けたとき、追加候補の選択・確定に関係するキーを処理する
	  @return true:キーを処理した  false:処理対象外
	*/
	bool OnKeyInput(unsigned int keyCode) override;
	/** 入力欄のキャレットまたは選択範囲変更通知を受けたとき、通常の検索中Stateへ戻る */
	void OnSelectionChanged() override;
	/** メインウインドウの位置またはサイズ変更通知を受けたとき、通常の検索中Stateへ戻る */
	void OnWindowGeometryChanged() override;
	/** 追加候補一覧の選択位置変更通知を受けたときの処理を行う */
	void OnExtraCandidateSelectionChanged() override;
	/** 追加候補一覧のクリック通知を受けたとき、選択中の追加候補を確定する */
	void OnExtraCandidateClicked() override;

private:
	int mPendingSelectionChangeNotifications{0};
};

}}}
