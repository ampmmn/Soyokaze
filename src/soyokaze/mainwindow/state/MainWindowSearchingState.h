#pragma once

#include "mainwindow/state/LauncherWindowStateBase.h"

namespace launcherapp { namespace mainwindow { namespace state {

/**
  メインウインドウが表示され、入力中の状態
*/
class SearchingState : public LauncherWindowStateBase
{
public:
	explicit SearchingState(LauncherWindowStateContextIF* context);

	/** 表示要求に応じて表示、再アクティブ化、トグル非表示を行う */
	void OnActivate(bool isShowForce) override;
	/** 非表示Stateへ遷移する */
	void OnDeactivate() override;
	/** 現在の候補を実行し、ウインドウが閉じた場合は非表示Stateへ遷移する */
	void OnExecuteRequested() override;
	/** 入力内容をクリアして待機中Stateへ戻る */
	void OnCancel() override;
	/** 入力内容がクリアされた結果に応じてStateを更新する */
	void OnContentCleared() override;
	/** 入力内容がなくなったら待機中Stateへ遷移する */
	void OnTextChanged() override;
	/** 検索結果を反映し、必要に応じてStateを更新する */
	void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
	/** 候補操作や実行に関係するキー入力を処理する
	  @return true:キーを処理した  false:処理対象外
	*/
	bool OnKeyInput(unsigned int keyCode) override;
	/** 候補の選択位置変更を反映する */
	void OnCandidateSelectionChanged(int index) override;
	/** クリックされた候補を入力欄へ反映する */
	void OnCandidateClicked() override;
	/** ダブルクリックされた候補を実行する */
	void OnCandidateDoubleClicked() override;
};

}}}
