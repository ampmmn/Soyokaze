#pragma once

#include "mainwindow/state/LauncherWindowStateBase.h"

namespace launcherapp { namespace mainwindow { namespace state {

/**
  メインウインドウが表示され、入力中の状態
*/
class SearchingState : public LauncherWindowStateBase
{
public:
	explicit SearchingState(LauncherWindowStateContextIF* context, bool allowParamSearching = true);

	/** ウインドウの表示要求を受けたとき、表示・再アクティブ化またはトグル非表示を行う */
	void OnActivate(bool isShowForce) override;
	/** ウインドウの非表示要求を受けたとき、非表示Stateへ遷移する */
	void OnDeactivate() override;
	/** 現在の候補の実行要求を受けたとき、実行し、ウインドウが閉じた場合は非表示Stateへ遷移する */
	void OnExecuteRequested() override;
	/** キャンセル操作を受けたとき、入力内容をクリアして待機中Stateへ戻る */
	void OnCancel() override;
	/** 入力内容がクリアされた通知を受けたとき、表示状態に応じてStateを更新する */
	void OnContentCleared() override;
	/** 入力欄の文字変更通知を受けたとき、入力内容がなくなれば待機中Stateへ遷移する */
	void OnTextChanged() override;
	/** 非同期検索の完了通知を受けたとき、検索結果を反映して必要に応じてStateを更新する */
	void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
	/** 入力欄でキー入力を受けたとき、候補操作や実行に関係するキーを処理する
	  @return true:キーを処理した  false:処理対象外
	*/
	bool OnKeyInput(unsigned int keyCode) override;
	/** 候補一覧の選択位置変更通知を受けたとき、選択位置を反映する */
	void OnCandidateSelectionChanged(int index) override;
	/** 候補一覧のクリック通知を受けたとき、クリックされた候補を入力欄へ反映する */
	void OnCandidateClicked() override;
	/** 候補一覧のダブルクリック通知を受けたとき、選択中の候補を実行する */
	void OnCandidateDoubleClicked() override;

private:
	bool mAllowParamSearching;
};

}}}
